// JsonConfigStore.h — nlohmann/json 的适配器（适配层，业务不可见）。
// 管理"配置目录"下按 section 分文件/分文件夹的 json：
//   Config/app.json                          （应用级设置）
//   Config/log.json                          （日志设置）
//   Config/node_management/node_config.json  （业务：节点管理）
//   Config/scheduled_tasks/task_config.json  （业务：定时任务）
// section id 直接作为相对路径（自动补 .json），因此 id 可以含 '/'
// 表示子文件夹。
#pragma once

#include "infra/config/IConfigStore.h"

#include <filesystem>
#include <memory>
#include <mutex>

namespace app::adapters{

class JsonConfigStore final : public app::infra::config::IConfigStore
    {
public:
    explicit JsonConfigStore(std::filesystem::path root_dir);
    ~JsonConfigStore() override;
    JsonConfigStore(const JsonConfigStore &) = delete;
    JsonConfigStore &operator=(const JsonConfigStore &) = delete;

    app::infra::config::FieldMap load(std::string_view section) const override;
    void save(std::string_view section,
              const app::infra::config::FieldMap &fields) override;
    // 目录式存储：save 即落盘，flush 为空操作
    void flush() override {}

private:
    // section 形如 "scheduled_tasks/task_config" ->
    // <root>/scheduled_tasks/task_config.json
    std::filesystem::path file_for(std::string_view section) const;

    std::filesystem::path root_dir_;
    mutable std::mutex mutex_;
    };

} // namespace app::adapters
