#include "core/AppSettingsSection.h"

#include "infra/logging/Log.h"

namespace app::core{

using namespace app::infra::config;

void AppSettingsSection::load_from(const IConfigStore &store)
    {
    const FieldMap m = store.load(id());
    data_.ui_style = field_or(m, "ui_style", data_.ui_style);
    data_.autostart = field_or(m, "autostart", data_.autostart);
    }

void AppSettingsSection::save_to(IConfigStore &store) const
    {
    FieldMap m;
    m["ui_style"] = data_.ui_style;
    m["autostart"] = data_.autostart;
    store.save(id(), m);
    }

std::vector<UiField> AppSettingsSection::ui_fields() const
    {
    return {
            {"ui_style", "界面风格", data_.ui_style},
            {"autostart", "开机启动", data_.autostart},
        };
    }

void AppSettingsSection::apply(const FieldMap &edits, IConfigStore &store)
    {
    data_.ui_style = field_or(edits, "ui_style", data_.ui_style);
    data_.autostart = field_or(edits, "autostart", data_.autostart);
    save_to(store);
    APP_LOG_INFO("Application settings updated");
    notify_change("应用设置已更新");
    }

} // namespace app::core
