// LogConfigSection.h — 日志自身也是一个配置段（sink 勾选、各 sink
// 级别、轮转参数）。文件: Config/log.json。由 UI
// 的日志设置对话框编辑，写回后触发日志重建。
#pragma once

#include "infra/config/IConfigSection.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace app::core{

struct LogSettings
    {
    bool ui_enabled = true;
    std::string ui_level = "info";

    bool file_enabled = true;
    std::string file_level = "debug";
    std::string file_path = "logs/app.log";
    std::int64_t file_max_size_mb = 5;
    std::int64_t file_rotate_count = 3;
    };

class LogConfigSection final : public app::infra::config::IConfigSection
    {
public:
    std::string_view id() const noexcept override { return "log"; }

    void load_from(const app::infra::config::IConfigStore &store) override;
    void save_to(app::infra::config::IConfigStore &store) const override;
    std::vector<app::infra::config::UiField> ui_fields() const override;
    void apply(const app::infra::config::FieldMap &edits,
               app::infra::config::IConfigStore &store) override;

    LogSettings &data() noexcept { return data_; }
    const LogSettings &data() const noexcept { return data_; }

private:
    LogSettings data_;
    };

    } // namespace app::core
