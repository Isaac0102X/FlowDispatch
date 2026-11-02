// AppSettingsSection.h — 应用级设置段（界面风格、开机启动）。文件:
// Config/app.json
#pragma once

#include "infra/config/IConfigSection.h"

#include <string>
#include <string_view>
#include <vector>

namespace app::core
    {

struct AppSettings
    {
    std::string ui_style = "Fusion";
    bool autostart = false;
    };

class AppSettingsSection final : public app::infra::config::IConfigSection
    {
public:
    std::string_view id() const noexcept override { return "app"; }

    void load_from(const app::infra::config::IConfigStore &store) override;
    void save_to(app::infra::config::IConfigStore &store) const override;
    std::vector<app::infra::config::UiField> ui_fields() const override;
    void apply(const app::infra::config::FieldMap &edits,
               app::infra::config::IConfigStore &store) override;

    AppSettings &data() noexcept { return data_; }
    const AppSettings &data() const noexcept { return data_; }

private:
    AppSettings data_;
    };

    } // namespace app::core
