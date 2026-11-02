// Bootstrapper.cpp — 启动顺序：
//   建配置存储(Config 目录) → 注册各业务配置段并加载 → 配置日志 sink
//   → 组装控制器与 UI → 进入事件循环。
#include "bootstrap/Bootstrapper.h"

#include "adapters/json/JsonConfigStore.h"
#include "adapters/logging/SinkFactory.h"
#include "adapters/qt/QtLogBridge.h"
#include "adapters/spdlog/SpdlogLogger.h"
#include "business/node_management/NodeConfigSection.h"
#include "business/scheduled_tasks/ScheduledTasksController.h"
#include "business/scheduled_tasks/TaskConfigSection.h"
#include "core/AppContext.h"
#include "core/AppSettingsSection.h"
#include "core/ConfigRegistry.h"
#include "core/LogConfigSection.h"
#include "infra/logging/Log.h"
//#include "ui/ConfigEditDialog.h"
//#include "ui/LogSettingsDialog.h"
#include "../ui/mainwindow.h"

#include <QApplication>
#include <QTimer>

#include <filesystem>
#include <memory>

namespace app::bootstrap{

using namespace app::infra::logging;
using namespace app::core;

int Bootstrapper::run()
    {
    // ---- 配置存储：Config 目录下按子文件夹/文件分门别类（见 CMakeLists
    // 复制规则）----
    auto store = std::make_shared<app::adapters::JsonConfigStore>("Config");

    // ---- 各业务/系统配置段自注册（开闭原则：新增业务=新增段+注册一行）----
    AppSettingsSection app;
    LogConfigSection log;
    app::business::node_management::NodeConfigSection node;
    app::business::scheduled_tasks::TaskConfigSection task;

    ConfigRegistry registry;
    registry.register_section(app);
    registry.register_section(log);
    registry.register_section(node);
    registry.register_section(task);
    registry.load_all(*store); // json -> 各段强类型

    // ---- 日志桥接（Qt 信号槽把任意线程的日志投递到 UI 线程）----
    app::adapters::qt::QtLogBridge log_bridge;

    // ---- 配置完成后按日志配置创建 UI / 文件 sink ----
    auto reload_logging = [&]
        {
        app::adapters::LogSinkSpec spec;
        spec.ui_enabled = log.data().ui_enabled;
        spec.ui_level = log.data().ui_level;
        spec.file_enabled = log.data().file_enabled;
        spec.file_level = log.data().file_level;
        spec.file_path = log.data().file_path;
        spec.file_max_size_mb = log.data().file_max_size_mb;
        spec.file_rotate_count = log.data().file_rotate_count;
        std::error_code ec;
        std::filesystem::create_directories(
            std::filesystem::path(spec.file_path).parent_path(), ec);
        Log::reconfigure(std::make_shared<app::adapters::SpdlogLogger>(
            app::adapters::SinkFactory::create(spec, log_bridge.callback())));
        };
    reload_logging();

    // ---- 业务 Facade：注入 MainWindow ----
    app::business::scheduled_tasks::ScheduledTasksController tasks(task);

    // ---- 装配上下文并显示主窗口 ----
    AppContext ctx;

    ctx.store = store.get();
    ctx.app = &app;
    ctx.log = &log;
    ctx.node = &node;
    ctx.task = &task;
    ctx.tasks = &tasks;
    ctx.log_bridge = &log_bridge;
    ctx.reload_logging = reload_logging;

    app::ui::MainWindow window(ctx);
    APP_LOG_INFO("logger online: ui={} file={}", log.data().ui_enabled,
                 log.data().file_enabled);
    window.show();

    const int rc = QApplication::exec();

    APP_LOG_INFO("=== app exiting, rc={} ===", rc);
    Log::shutdown();
    return rc;
    }

} // namespace app::bootstrap
