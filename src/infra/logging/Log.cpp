// Log.cpp — 门面实现。真正的 logger 由 Bootstrapper 经 SinkFactory 创建后注入。
#include "infra/logging/Log.h"

#include <utility>

namespace app::infra::logging
    {

namespace{

class NullLogger final : public ILogger
    {
public:
    void log(Level, std::string_view, const std::source_location &) override {}
    bool enabled(Level) const noexcept override { return false; }
    void flush() override {}
    };

std::shared_ptr<ILogger> &slot()
    {
    static std::shared_ptr<ILogger> s = std::make_shared<NullLogger>();
    return s;
    }

} // namespace

void Log::reconfigure(std::shared_ptr<ILogger> logger)
    {
    slot() = std::move(logger);
    }

void Log::shutdown()
    {
    slot()->flush();
    slot() = std::make_shared<NullLogger>();
    }

ILogger &Log::get()
    {
    return *slot();
    }

void Log::debug(std::string_view msg, const std::source_location &loc)
    {
    get().log(Level::debug, msg, loc);
    }
void Log::info(std::string_view msg, const std::source_location &loc)
    {
    get().log(Level::info, msg, loc);
    }
void Log::warn(std::string_view msg, const std::source_location &loc)
    {
    get().log(Level::warn, msg, loc);
    }
void Log::error(std::string_view msg, const std::source_location &loc)
    {
    get().log(Level::error, msg, loc);
    }
void Log::critical(std::string_view msg, const std::source_location &loc)
    {
    get().log(Level::critical, msg, loc);
    }

} // namespace app::infra::logging
