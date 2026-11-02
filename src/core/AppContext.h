// AppContext.h — 应用装配上下文：把 store / 配置段 / 控制器 / 日志桥接注入 UI。
// 只存指针与前向声明，避免 core 依赖 Qt 头文件。
#pragma once

#include "infra/config/IConfigSection.h"
#include "infra/config/IConfigStore.h"

#include <functional>

namespace app::business::node_management { class NodeConfigSection; }
namespace app::business::scheduled_tasks { class TaskConfigSection; }
namespace app::business::scheduled_tasks { class ScheduledTasksController; }
namespace app::adapters::qt { class QtLogBridge; }
namespace app::core { class AppSettingsSection; }
namespace app::core { class LogConfigSection; }

namespace app::core {

struct AppContext {
    app::infra::config::IConfigStore* store = nullptr;

    AppSettingsSection* app = nullptr;
    LogConfigSection* log = nullptr;
    app::business::node_management::NodeConfigSection* node = nullptr;
    app::business::scheduled_tasks::TaskConfigSection* task = nullptr;

    app::business::scheduled_tasks::ScheduledTasksController* tasks = nullptr;
    app::adapters::qt::QtLogBridge* log_bridge = nullptr;

    // 应用日志配置：读 LogConfigSection -> SinkFactory -> Log::reconfigure
    std::function<void()> reload_logging;
};

} // namespace app::core
