// JsonConfigStore.cpp — nlohmann/json 实现。全工程唯一直接 include nlohmann
// 的地方。
#include "adapters/json/JsonConfigStore.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <type_traits>

namespace app::adapters
    {

using json = nlohmann::json;
using namespace app::infra::config;

JsonConfigStore::JsonConfigStore(std::filesystem::path root_dir)
    : root_dir_(std::move(root_dir))
    {
    }
JsonConfigStore::~JsonConfigStore() = default;

std::filesystem::path JsonConfigStore::file_for(std::string_view section) const
    {
    // section 形如 "scheduled_tasks/task_config" ->
    // <root>/scheduled_tasks/task_config.json
    std::filesystem::path p(root_dir_);
    p /= std::filesystem::path(std::string(section) + ".json");
    return p;
    }

// ---- 中立 FieldMap <-> json 转换（唯一接触 nlohmann 的地方）----
namespace
    {
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
    } // namespace

FieldMap JsonConfigStore::load(std::string_view section) const
    {
    std::lock_guard<std::mutex> lk(mutex_);
    const auto path = file_for(section);
    std::ifstream ifs(path);
    if (!ifs)
        return {}; // 首次运行：文件不存在，视为空配置（业务回退默认值）
    //TODO:考虑移到外层去
    try
        {
        const json root = json::parse(ifs);
        return json_to_fields(root);
        }
    catch (const std::exception &)
        {
        return {};
        }
    }

void JsonConfigStore::save(std::string_view section, const FieldMap &fields)
    {
    std::lock_guard<std::mutex> lk(mutex_);
    const auto path = file_for(section);
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    std::ofstream ofs(path);
    if (ofs)
        ofs << fields_to_json(fields).dump(2) << "\n";
    }

    } // namespace app::adapters
