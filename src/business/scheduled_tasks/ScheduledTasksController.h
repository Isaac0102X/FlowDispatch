// ScheduledTasksController.h — scheduled_tasks 业务包的控制器（Facade）。
// 注入 MainWindow：演示"业务类也能做配置、写日志、融入统一架构"。
#pragma once

#include "business/scheduled_tasks/TaskConfigSection.h"

#include <string>

namespace app::business::scheduled_tasks {

class ScheduledTasksController {
public:
    explicit ScheduledTasksController(TaskConfigSection& cfg);

    // 模拟启动/停止调度引擎
    void start();
    void stop();
    bool is_running() const noexcept { return running_; }

    // 演示动作：读配置 + 写日志（std::format + 调用点）
    void run_once();

    TaskConfigSection &config() noexcept { return cfg_; }

private:
    TaskConfigSection &cfg_;
    bool running_ = false;
    unsigned run_count_ = 0;
    };

} // namespace app::business::scheduled_tasks
