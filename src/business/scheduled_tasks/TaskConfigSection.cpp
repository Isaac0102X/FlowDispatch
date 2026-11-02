// TaskConfigSection.cpp
#include "business/scheduled_tasks/TaskConfigSection.h"

namespace app::business::scheduled_tasks {

using namespace app::infra::config;

void TaskConfigSection::load_from(const IConfigStore &store)
    {
    const FieldMap m = store.load(id());
    data_.cron_expr = field_or(m, "cron_expr", data_.cron_expr);
    data_.retry_count = field_or(m, "retry_count", data_.retry_count);
    data_.enabled = field_or(m, "enabled", data_.enabled);
    }

void TaskConfigSection::save_to(IConfigStore &store) const
    {
    FieldMap m;
    m["cron_expr"] = data_.cron_expr;
    m["retry_count"] = static_cast<std::int64_t>(data_.retry_count);
    m["enabled"] = data_.enabled;
    store.save(id(), m);
    }

std::vector<UiField> TaskConfigSection::ui_fields() const
    {
    return {
        {"cron_expr", "Cron 表达式", data_.cron_expr},
        {"retry_count", "重试次数",
         static_cast<std::int64_t>(data_.retry_count)},
        {"enabled", "启用", data_.enabled},
    };
    }

void TaskConfigSection::apply(const FieldMap &edits, IConfigStore &store)
    {
    data_.cron_expr = field_or(edits, "cron_expr", data_.cron_expr);
    data_.retry_count = field_or(edits, "retry_count", data_.retry_count);
    data_.enabled = field_or(edits, "enabled", data_.enabled);
    save_to(store);
    notify_change("定时任务配置已更新");
    }

} // namespace app::business::scheduled_tasks
