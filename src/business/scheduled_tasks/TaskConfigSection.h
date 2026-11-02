// TaskConfigSection.h — scheduled_tasks 业务包自持的配置段（模式与
// node_management 相同）。
#pragma once

#include "infra/config/IConfigSection.h"

#include <string>
#include <string_view>
#include <vector>

namespace app::business::scheduled_tasks {

struct TaskConfig {
    std::string cron_expr = "0 * * * *";
    int retry_count = 3;
    bool enabled = true;
    };

class TaskConfigSection final : public app::infra::config::IConfigSection
    {
public:
    std::string_view id() const noexcept override
        {
        return "scheduled_tasks/task_config";
        }

    void load_from(const app::infra::config::IConfigStore &store) override;
    void save_to(app::infra::config::IConfigStore &store) const override;
    std::vector<app::infra::config::UiField> ui_fields() const override;
    void apply(const app::infra::config::FieldMap &edits,
               app::infra::config::IConfigStore &store) override;

    TaskConfig &data() noexcept { return data_; }
    const TaskConfig &data() const noexcept { return data_; }

private:
    TaskConfig data_;
    };

} // namespace app::business::scheduled_tasks
