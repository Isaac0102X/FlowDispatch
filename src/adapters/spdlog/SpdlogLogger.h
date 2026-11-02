// SpdlogLogger.h — spdlog 的日志适配器（适配层，业务不可见）。
// 头文件不 include spdlog，前向声明 spdlog::logger。
#pragma once

#include "infra/logging/Log.h"

#include <memory>

namespace spdlog
    {
class logger;
    }

namespace app::adapters{

class SpdlogLogger final : public app::infra::logging::ILogger
    {
    public:
        explicit SpdlogLogger(std::shared_ptr<spdlog::logger> logger);
        ~SpdlogLogger() override;

        void log(app::infra::logging::Level level, std::string_view msg,
                 const std::source_location &loc) override;
        bool enabled(app::infra::logging::Level level) const noexcept override;
        void flush() override;

    private:
        std::shared_ptr<spdlog::logger> logger_;
    };

} // namespace app::adapters
