#include "core/LogConfigSection.h"

#include "infra/logging/Log.h"

namespace app::core{

using namespace app::infra::config;

void LogConfigSection::load_from(const IConfigStore &store)
    {
    const FieldMap m = store.load(id());
    data_.ui_enabled = field_or(m, "ui_enabled", data_.ui_enabled);
    data_.ui_level = field_or(m, "ui_level", data_.ui_level);
    data_.file_enabled = field_or(m, "file_enabled", data_.file_enabled);
    data_.file_level = field_or(m, "file_level", data_.file_level);
    data_.file_path = field_or(m, "file_path", data_.file_path);
    data_.file_max_size_mb =
        field_or(m, "file_max_size_mb", data_.file_max_size_mb);
    data_.file_rotate_count =
        field_or(m, "file_rotate_count", data_.file_rotate_count);
    }

void LogConfigSection::save_to(IConfigStore &store) const
    {
    FieldMap m;
    m["ui_enabled"] = data_.ui_enabled;
    m["ui_level"] = data_.ui_level;
    m["file_enabled"] = data_.file_enabled;
    m["file_level"] = data_.file_level;
    m["file_path"] = data_.file_path;
    m["file_max_size_mb"] = data_.file_max_size_mb;
    m["file_rotate_count"] = data_.file_rotate_count;
    store.save(id(), m);
    }

std::vector<UiField> LogConfigSection::ui_fields() const
    {
    return {
        {"ui_enabled", "UI 日志面板", data_.ui_enabled},
        {"ui_level", "UI 面板级别", data_.ui_level},
        {"file_enabled", "轮转文件", data_.file_enabled},
        {"file_level", "文件级别", data_.file_level},
        {"file_path", "文件路径", data_.file_path},
        {"file_max_size_mb", "单文件大小(MB)", data_.file_max_size_mb},
        {"file_rotate_count", "保留文件数", data_.file_rotate_count},
    };
    }

void LogConfigSection::apply(const FieldMap &edits, IConfigStore &store)
    {
    data_.ui_enabled = field_or(edits, "ui_enabled", data_.ui_enabled);
    data_.ui_level = field_or(edits, "ui_level", data_.ui_level);
    data_.file_enabled = field_or(edits, "file_enabled", data_.file_enabled);
    data_.file_level = field_or(edits, "file_level", data_.file_level);
    data_.file_path = field_or(edits, "file_path", data_.file_path);
    data_.file_max_size_mb =
        field_or(edits, "file_max_size_mb", data_.file_max_size_mb);
    data_.file_rotate_count =
        field_or(edits, "file_rotate_count", data_.file_rotate_count);
    save_to(store);
    APP_LOG_INFO("Log settings updated");
    notify_change("日志设置已更新");
    }

} // namespace app::core
