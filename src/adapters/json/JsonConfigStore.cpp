// JsonConfigStore.cpp — nlohmann/json 实现。全工程唯一直接 include nlohmann
// 的地方。 具体设计理由（模板/用户分离、合并规则、原子写）见同名 .h
// 的顶部说明。
#include "adapters/json/JsonConfigStore.h"

#include "infra/logging/Log.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>

#ifdef _WIN32
// Windows 的原子替换需要 Win32 API（std::filesystem::rename
// 不能覆盖已存在文件）
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace app::adapters{

namespace fs = std::filesystem;
using json = nlohmann::json;
using namespace app::infra::config;

JsonConfigStore::JsonConfigStore(fs::path config_dir)
    : root_dir_(std::move(config_dir))
    {
    }
JsonConfigStore::~JsonConfigStore() = default;

fs::path JsonConfigStore::section_base(std::string_view section) const
    {
    // section 形如
    // "scheduled_tasks/task_config"，拼接后作为相对路径保留子目录层级
    return root_dir_ / fs::path(std::string(section));
    }

fs::path JsonConfigStore::user_file(std::string_view section) const
    {
    return fs::path(section_base(section).string() + ".json");
    }

fs::path JsonConfigStore::template_file(std::string_view section) const
    {
    return fs::path(section_base(section).string() + ".default.json");
    }

// ---- 中立 FieldMap <-> json 转换（唯一接触 nlohmann 结构的地方）----
namespace{

FieldMap json_to_fields(const json &obj)
    {
    FieldMap fm;
    if (!obj.is_object())
        return fm;
    for (auto it = obj.begin(); it != obj.end(); ++it)
        {
        const json &v = it.value();
        if (v.is_boolean())
            fm[it.key()] = v.get<bool>();
        else if (v.is_number_integer())
            fm[it.key()] = v.get<std::int64_t>();
        else if (v.is_number_float())
            fm[it.key()] = v.get<double>();
        else if (v.is_string())
            fm[it.key()] = v.get<std::string>();
        else if (v.is_null())
            fm[it.key()] = FieldValue{};
        else
            fm[it.key()] =
                v.dump(); // 嵌套对象/数组降级为字符串（当前业务全标量）
        }
    return fm;
    }

json fields_to_json(const FieldMap &fm)
    {
    json obj = json::object();
    for (const auto &[k, fv] : fm)
        {
        std::visit([&](const auto &v)
        {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::nullptr_t>)
                obj[k] = nullptr;
            else if constexpr (std::is_same_v<T, bool>)
                obj[k] = v;
            else if constexpr (std::is_same_v<T, std::int64_t>)
                obj[k] = v;
            else if constexpr (std::is_same_v<T, double>)
                obj[k] = v;
            else
                obj[k] = v; // std::string
        }, fv.v);
        }
    return obj;
    }

// 读取原始 json 对象（用于写回时保留未提交字段的原始结构）。
// 文件不存在是常态（首次运行 / 该段尚无用户配置），返回空对象即可，不打日志。
json read_json_object(const fs::path &path)
    {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs)
        return json::object();
    try
        {
        json j = json::parse(ifs);
        return j.is_object() ? j : json::object();
        }
    catch (const std::exception &e)
        {
        // 文件损坏（例如手工编辑出错）：本次按空对象继续启动，保证程序可用；
        // 下次写回会用合法内容替换该文件（此处显式告警，便于支持人员定位）。
        APP_LOG_WARN("配置文件解析失败，本次按空配置处理：{}（{}）",
            path.string(), e.what());
        return json::object();
        }
    }

FieldMap read_fields(const fs::path &path)
    {
    return json_to_fields(read_json_object(path));
    }

// 原子写文本：写 <target>.tmp -> 关闭（数据落到 OS）-> 原子替换 target。
// 崩溃安全性：任何时刻正式文件要么是旧的完整内容，要么是新的完整内容，不会出现半截
// JSON。
bool atomic_write_text(const fs::path &target, const std::string &text)
    {
    const fs::path tmp = fs::path(target.string() + ".tmp");

        // 1) 写临时文件
        {
        std::ofstream ofs(tmp, std::ios::binary | std::ios::trunc);
        if (!ofs)
            {
            APP_LOG_ERROR("无法创建临时配置文件：{}", tmp.string());
            return false;
            }
        ofs.write(text.data(), static_cast<std::streamsize>(text.size()));
        ofs.flush();
        if (!ofs)
            {
            ofs.close();
            std::error_code ec;
            fs::remove(tmp, ec);
            APP_LOG_ERROR("写入临时配置文件失败：{}", tmp.string());
            return false;
            }
        }

    // 2) 原子替换（平台差异在这里，其余逻辑完全一致）
#ifdef _WIN32
    // std::filesystem::rename 在 Windows 上不能覆盖已存在文件，用 MoveFileExW：
    // MOVEFILE_REPLACE_EXISTING 原子替换；MOVEFILE_WRITE_THROUGH
    // 等待落盘再返回。
    if (!::MoveFileExW(tmp.native().c_str(), target.native().c_str(),
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        {
        const unsigned long err = ::GetLastError();
        std::error_code ec;
        fs::remove(tmp, ec); // 清理临时文件，不留下垃圾
        APP_LOG_ERROR(
            "原子替换配置文件失败（Win32 错误 {}）：{}", err, target.string());
        return false;
        }
#else
    // POSIX：同一目录内 rename 覆盖同名文件是原子操作
    std::error_code ec;
    fs::rename(tmp, target, ec);
    if (ec)
        {
        std::error_code ec2;
        fs::remove(tmp, ec2);
        APP_LOG_ERROR(
            "原子替换配置文件失败：{}（{}）", target.string(), ec.message());
        return false;
        }
#endif
    return true;
    }

} // namespace

FieldMap JsonConfigStore::load(std::string_view section) const
    {
    std::lock_guard<std::mutex> lk(mutex_);

    // 低优先级：出厂模板（v1.1 新增字段的默认值就写在这里，随包分发）
    FieldMap merged = read_fields(template_file(section));

    // 高优先级：用户配置（只覆盖用户显式写过的字段，其余保留模板默认）
    // 这正是"老用户 config.json
    // 缺新字段"的解法：字段级合并，而不是整文件取其一。
    for (const auto &[k, v] : read_fields(user_file(section)))
        merged[k] = v;

    return merged; // 模板也没有 ->
                   // 空表，由配置段内置默认值兜底（开发环境无需模板）
    }

void JsonConfigStore::save(std::string_view section, const FieldMap &fields)
    {
    std::lock_guard<std::mutex> lk(mutex_);

    const fs::path path = user_file(section);
    std::error_code ec;
    fs::create_directories(path.parent_path(), ec);

    // 1) 读现有用户文件的原始 json：保留本次没提交的字段
    //    （旧版本字段、手工添加字段、未知嵌套结构都不丢；出厂模板从不参与写回）
    json obj = read_json_object(path);

    // 2) 只覆盖本次提交的键
    const json patch = fields_to_json(fields);
    for (auto &[k, v] : patch.items())
        obj[k] = v;

    // 3) 原子替换，避免崩溃留下半截 JSON
    atomic_write_text(path, obj.dump(2) + "\n");
    }

bool JsonConfigStore::exists(std::string_view section) const
    {
    std::lock_guard<std::mutex> lk(mutex_);
    std::error_code ec;
    return fs::exists(user_file(section), ec);
    }

} // namespace app::adapters
