// SinkFactory.h — 按日志配置（来自 LogConfigSection）创建 spdlog logger
// 的工厂。适配层，业务不可见。支持：UI 面板 / 轮转文件，每个 sink 独立级别。
#pragma once

#include "infra/logging/Log.h"

#include <functional>
#include <memory>
#include <string>

namespace spdlog{
class logger;
}

namespace app::adapters{

// 由 LogConfigSection 填充；级别用字符串（"trace".."off"）与 UI/JSON 一致
struct LogSinkSpec
    {
    bool ui_enabled = true;
    std::string ui_level = "info";

    bool file_enabled = true;
    std::string file_level = "debug";
    std::string file_path = "logs/app.log";
    std::int64_t file_max_size_mb = 5;
    std::int64_t file_rotate_count = 3;

    std::string pattern = "[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v";
    };

class SinkFactory
    {
    public:
        using UiCallback = std::function<void(const app::infra::logging::Record &)>;

        // ui_cb 由 UI 层注册（Qt 信号槽），日志层不依赖 Qt。
        // 回调同步触发；如需异步请自行拷贝 Record 再投递。
        static std::shared_ptr<spdlog::logger> create(const LogSinkSpec &spec,
                                                      UiCallback ui_cb = {});
    };

} // namespace app::adapters
