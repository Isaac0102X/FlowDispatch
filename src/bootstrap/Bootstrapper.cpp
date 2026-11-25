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
#include "AppContext.h"
#include "cfg_management/ConfigRegistry.h"
#include "cfg_management/BasicSettings/LogConfigSection.h"
#include "cfg_management/BasicSettings/CommonSection.h"
#include "infra/logging/Log.h"
#include "../ui/mainwindow.h"

#include <QApplication>
#include <QTimer>

#include <filesystem>
#include <memory>
#include <cassert>

namespace app::assembly{

using namespace app::infra::logging;
using namespace app::cfg_management;

namespace{

std::filesystem::path resolve_config_dir()
    {
    if (const char *env = std::getenv("APP_CONFIG_DIR");
        env != nullptr && *env != '\0')
        {
        return std::filesystem::path(env);
        }
    return std::filesystem::path(
               QCoreApplication::applicationDirPath().toStdString()) /
           "Config";
    }

// 配置里的相对路径（如日志 "logs/app.log"）统一解析到 exe 目录，
// 保证绿色版的日志、配置文件都跟着程序走，而不是跟着当前工作目录跑。
// 返回空 path 表示失败，调用方决定怎么处理
std::filesystem::path resolve_runtime_path(const std::string& configured)
    {
    if (configured.empty())
        return {};

    std::filesystem::path p(configured);
    if (!p.is_absolute())
        p = std::filesystem::path(
                QCoreApplication::applicationDirPath().toStdString()) / p;

    std::error_code ec;
    auto r = std::filesystem::weakly_canonical(p, ec);
    return ec ? std::filesystem::path{} : r;
    }

} // namespace

int Bootstrapper::run()
    {
    // 配置存储：<exe目录>/Config；模板(*.default.json)与用户配置(*.json)分离
    // ---- 布局与升级语义见 adapters/json/JsonConfigStore.h 顶部说明。
    const auto config_dir = resolve_config_dir();
    auto store = std::make_shared<::app::adapters::JsonConfigStore>(config_dir);

    // ---- 各业务/系统配置段自注册（开闭原则：新增业务=新增段+注册一行）----
    LogConfigSection log;
    ::app::business::node_management::NodeConfigSection node;
    ::app::business::scheduled_tasks::TaskConfigSection task;
    app::cfg_management::BasicSettings::CommonSection common;

    ConfigRegistry registry;
    registry.register_section(log);
    registry.register_section(node);
    registry.register_section(task);
    registry.register_section(common);
    registry.load_all(*store); // 模板 + 用户配置按字段合并 -> 各段强类型

    // 首次运行：为还没有用户配置的段生成一份完整、可编辑的 *.json；
    // 已存在的用户配置一律不覆盖（绿色版解压覆盖升级不会丢用户数据）。
    registry.save_missing(*store);

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
        // 日志相对路径统一相对 exe 目录（绿色版所有落盘产物都跟着程序走）
        spec.file_path = resolve_runtime_path(log.data().file_path);
        assert(!spec.file_path.empty());

        spec.file_max_size_mb = log.data().file_max_size_mb;
        spec.file_rotate_count = log.data().file_rotate_count;

        spec.flush_level = log.data().flush_level;
        spec.flush_every_sec = log.data().flush_every_sec;

        std::error_code ec;
        std::filesystem::create_directories(
            std::filesystem::path(spec.file_path).parent_path(), ec);
        Log::reconfigure(std::make_shared<::app::adapters::SpdlogLogger>(
            ::app::adapters::SinkFactory::create(spec, log_bridge.callback())));
        };

    reload_logging();

    // ---- 业务 Facade：注入 MainWindow ----
    app::business::scheduled_tasks::ScheduledTasksController tasks(task);

    // ---- 装配上下文并显示主窗口 ----
    AppContext ctx;

    ctx.store = store.get();
    ctx.log = &log;
    ctx.node = &node;
    ctx.task = &task;
    ctx.tasks = &tasks;
    ctx.common = &common;
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

} // namespace app::assembly
