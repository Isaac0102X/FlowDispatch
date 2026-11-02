#include "business/scheduled_tasks/ScheduledTasksController.h"

#include "infra/logging/Log.h"

namespace app::business::scheduled_tasks
    {

ScheduledTasksController::ScheduledTasksController(TaskConfigSection &cfg)
    : cfg_(cfg)
    {
    }

void ScheduledTasksController::start()
    {
    running_ = true;
    APP_LOG_INFO("ScheduledTasksController started, cron='{}', enabled={}",
                 cfg_.data().cron_expr, cfg_.data().enabled);
    }

void ScheduledTasksController::stop()
    {
    running_ = false;
    APP_LOG_INFO("ScheduledTasksController stopped");
    }

void ScheduledTasksController::run_once()
    {
    ++run_count_;
    // 读配置（业务强类型，来自 json -> UI -> 写回链路）
    const auto &c = cfg_.data();
    APP_LOG_INFO("run_once #{} | cron='{}' retry={} enabled={}", run_count_,
                 c.cron_expr, c.retry_count, c.enabled);
    // 这里即业务逻辑本体（示例留空）
    if (!c.enabled)
        {
        APP_LOG_WARN("task disabled, skip execution");
        return;
        }
    }

    } // namespace app::business::scheduled_tasks
