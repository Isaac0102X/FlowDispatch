// SpdlogLogger.cpp — 把 spdlog 适配成 ILogger。这是全工程唯一直接 include
// spdlog 的日志转换处。
#include "adapters/spdlog/SpdlogLogger.h"

#include <spdlog/spdlog.h>

#include <utility>

namespace app::adapters{

using app::infra::logging::Level;

SpdlogLogger::SpdlogLogger(std::shared_ptr<spdlog::logger> logger)
    : logger_(std::move(logger))
    {
    }
SpdlogLogger::~SpdlogLogger() = default;

void SpdlogLogger::log(Level level, std::string_view msg,
                       const std::source_location &loc)
    {
    if (!logger_)
        return;
    spdlog::source_loc sloc(loc.file_name(), static_cast<int>(loc.line()), "");
    switch (level)
        {
    case Level::trace:
        logger_->log(sloc, spdlog::level::trace, "{}", msg);
        break;
    case Level::debug:
        logger_->log(sloc, spdlog::level::debug, "{}", msg);
        break;
    case Level::info:
        logger_->log(sloc, spdlog::level::info, "{}", msg);
        break;
    case Level::warn:
        logger_->log(sloc, spdlog::level::warn, "{}", msg);
        break;
    case Level::error:
        logger_->log(sloc, spdlog::level::err, "{}", msg);
        break;
    case Level::critical:
        logger_->log(sloc, spdlog::level::critical, "{}", msg);
        break;
        }
    }

bool SpdlogLogger::enabled(Level level) const noexcept
    {
    return logger_ &&
           logger_->should_log(static_cast<spdlog::level::level_enum>(level));
    }

void SpdlogLogger::flush()
    {
    if (logger_)
        logger_->flush();
    }

} // namespace app::adapters
