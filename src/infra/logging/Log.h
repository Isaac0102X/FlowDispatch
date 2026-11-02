// Log.h — 日志抽象 + 门面（业务逻辑唯一需要 include 的日志头）。
//
// 设计要点（与 UML 包图架构一致，C++20）：
//  * 业务只见 ILogger / ISink / Log 门面，绝不 include <spdlog>。
//  * Log 门面是"静态全局访问点"，但不是全局可变对象：持有
//  std::shared_ptr<ILogger>，
//    实现由 Bootstrapper 注入，测试可注入 fake。任何模块任何位置都能写日志。
//  * 配置加载后由 Bootstrapper 注入 logger；此前日志不输出。
//  * 类型安全格式化使用 C++20 std::format（编译期校验格式串）；调用点用
//  std::source_location。
#pragma once

#include <chrono>
#include <format>
#include <memory>
#include <source_location>
#include <string>
#include <string_view>
#include <utility>

namespace app::infra::logging
    {

enum class Level : int
    {
    trace = 0,
    debug,
    info,
    warn,
    error,
    critical,
    off
    };

inline std::string_view level_name(Level l) noexcept
    {
    switch (l)
        {
    case Level::trace:
        return "trace";
    case Level::debug:
        return "debug";
    case Level::info:
        return "info";
    case Level::warn:
        return "warn";
    case Level::error:
        return "error";
    case Level::critical:
        return "critical";
    case Level::off:
        return "off";
        }
    return "?";
    }

// 一条日志记录（sink 收到的）
struct Record
    {
    Level level;
    std::string_view message;
    std::string_view file;
    unsigned line;
    std::chrono::system_clock::time_point time;
    };

// 输出目标抽象（适配器目标）：轮转文件 / UI（Qt）
class ISink
    {
public:
    virtual ~ISink() = default;
    virtual void write(const Record &rec) = 0;
    virtual Level min_level() const noexcept = 0;
    };

// 日志后端抽象（适配器目标）：spdlog 只是其中一种实现
class ILogger
    {
public:
    virtual ~ILogger() = default;
    virtual void
    log(Level level, std::string_view msg,
        const std::source_location &loc = std::source_location::current()) = 0;
    virtual bool enabled(Level level) const noexcept = 0;
    virtual void flush() = 0;
    };

// 门面：静态全局访问点（非全局可变对象）
class Log
    {
public:
    // 配置就绪后，注入按配置构建的 logger
    static void reconfigure(std::shared_ptr<ILogger> logger);
    static void shutdown();

    static ILogger &get();

    // 非模板：接收已格式化字符串（零格式化依赖），不传 source_location 时，默认参数会记录调用它的位置
    static void
    debug(std::string_view msg,
          const std::source_location &loc = std::source_location::current());
    static void
    info(std::string_view msg,
         const std::source_location &loc = std::source_location::current());
    static void
    warn(std::string_view msg,
         const std::source_location &loc = std::source_location::current());
    static void
    error(std::string_view msg,
          const std::source_location &loc = std::source_location::current());
    static void
    critical(std::string_view msg,
             const std::source_location &loc = std::source_location::current());
/*
    当前 source_location::current() 写在模板函数体里，因此记录的位置会落在 Log.h 中，而不是调用处
    这里有待删掉
    // C++20 std::format：业务只写 Log::info("x={}", v)，编译期校验格式串
    template <typename... Args>
    static void debug(std::format_string<Args...> fmt, Args &&...args)
        {
        logv(Level::debug, std::source_location::current(), fmt,
             std::forward<Args>(args)...);
        }
    template <typename... Args>
    static void info(std::format_string<Args...> fmt, Args &&...args)
        {
        logv(Level::info, std::source_location::current(), fmt,
             std::forward<Args>(args)...);
        }
    template <typename... Args>
    static void warn(std::format_string<Args...> fmt, Args &&...args)
        {
        logv(Level::warn, std::source_location::current(), fmt,
             std::forward<Args>(args)...);
        }
    template <typename... Args>
    static void error(std::format_string<Args...> fmt, Args &&...args)
        {
        logv(Level::error, std::source_location::current(), fmt,
             std::forward<Args>(args)...);
        }
    template <typename... Args>
    static void critical(std::format_string<Args...> fmt, Args &&...args)
        {
        logv(Level::critical, std::source_location::current(), fmt,
             std::forward<Args>(args)...);
        }
*/
    // 显式携带调用点（配合下方宏使用，能拿到业务真实调用行；
    // 便捷 info(fmt, args...) 的调用点会落在 Log.h，因为 current()
    // 在模板内求值）。
    template <typename... Args>
    static void at(Level lv, const std::source_location &loc,
                   std::format_string<Args...> fmt, Args &&...args)
        {
        get().log(lv, std::format(fmt, std::forward<Args>(args)...), loc);
        }

private:
    template <typename... Args>
    static void logv(Level lv, const std::source_location &loc,
                     std::format_string<Args...> fmt, Args &&...args)
        {
        get().log(lv, std::format(fmt, std::forward<Args>(args)...), loc);
        }
    static std::shared_ptr<ILogger> &impl();
    };

    } // namespace app::infra::logging

// 调用点正确的便捷宏（与 spdlog 做法一致）：宏展开处
// std::source_location::current() 即业务调用行
#define APP_LOG_DEBUG(...)                                                     \
    ::app::infra::logging::Log::at(::app::infra::logging::Level::debug,        \
                                   ::std::source_location::current(),          \
                                   __VA_ARGS__)
#define APP_LOG_INFO(...)                                                      \
    ::app::infra::logging::Log::at(::app::infra::logging::Level::info,         \
                                   ::std::source_location::current(),          \
                                   __VA_ARGS__)
#define APP_LOG_WARN(...)                                                      \
    ::app::infra::logging::Log::at(::app::infra::logging::Level::warn,         \
                                   ::std::source_location::current(),          \
                                   __VA_ARGS__)
#define APP_LOG_ERROR(...)                                                     \
    ::app::infra::logging::Log::at(::app::infra::logging::Level::error,        \
                                   ::std::source_location::current(),          \
                                   __VA_ARGS__)
#define APP_LOG_CRITICAL(...)                                                  \
    ::app::infra::logging::Log::at(::app::infra::logging::Level::critical,     \
                                   ::std::source_location::current(),          \
                                   __VA_ARGS__)
