
#pragma once

#include "infra/config/IConfigStore.h"

#include <filesystem>
#include <mutex>

namespace app::adapters{

class JsonConfigStore final : public app::infra::config::IConfigStore
    {
public:
    // config_dir：配置根目录（绿色版为 <exe目录>/Config）
    explicit JsonConfigStore(std::filesystem::path config_dir);
    ~JsonConfigStore() override;
    JsonConfigStore(const JsonConfigStore &) = delete;
    JsonConfigStore &operator=(const JsonConfigStore &) = delete;

    // 读取：模板（低优先级）+ 用户配置（高优先级）按字段合并
    app::infra::config::FieldMap load(std::string_view section) const override;

    // 写回：只写用户文件，保留未知字段，原子替换
    void save(std::string_view section,
        const app::infra::config::FieldMap &fields) override;

    // 用户配置文件（*.json）是否已存在；出厂模板不算
    bool exists(std::string_view section) const override;

    // 目录式存储：save 已即时落盘，flush 无需额外动作
    void flush() override {}

    // 配置根目录（供启动日志展示，便于用户/支持人员确认读的是哪份配置）
    const std::filesystem::path &root_dir() const noexcept { return root_dir_; }

private:
    // <root>/<section>.json —— 用户配置
    std::filesystem::path user_file(std::string_view section) const;
    // <root>/<section>.default.json —— 出厂模板
    std::filesystem::path template_file(std::string_view section) const;
    // 由 section 拼出 "<root>/<section>" 的公共部分（不含扩展名）
    std::filesystem::path section_base(std::string_view section) const;

    std::filesystem::path root_dir_;
    mutable std::mutex mutex_; // 配置读写是低频操作，一把互斥锁足够，简单可靠
    };

} // namespace app::adapters
