// SinkFactory.cpp — 组装 spdlog sink（UI 面板 / 轮转文件）。这里
// include spdlog（适配层）。
#include "adapters/logging/SinkFactory.h"

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <mutex>
#include <utility>
#include <vector>

namespace app::adapters{

using namespace app::infra::logging;

namespace{
// "warn" -> Level::warn，未知回退 info
Level level_from_name(const std::string &s)
    {
    if (s == "trace")
        return Level::trace;
    if (s == "debug")
        return Level::debug;
    if (s == "info")
        return Level::info;
    if (s == "warn")
        return Level::warn;
    if (s == "err" || s == "error")
        return Level::error;
    if (s == "critical")
        return Level::critical;
    if (s == "off")
        return Level::off;
    return Level::info;
    }

// 把 spdlog 的输出转发给 UI 层回调（不依赖 Qt，桥接在 UI 层做）
class UiSink final : public spdlog::sinks::base_sink<std::mutex>
    {
public:
    explicit UiSink(SinkFactory::UiCallback cb) : cb_(std::move(cb)) {}

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override
        {
        if (!cb_)
            return;
        Record r;
        r.level = static_cast<Level>(msg.level);
        r.message = std::string_view(msg.payload.data(), msg.payload.size());
        r.file = msg.source.filename ? std::string_view(msg.source.filename)
                                     : std::string_view{};
        r.line = static_cast<unsigned>(msg.source.line);
        r.time = std::chrono::system_clock::now();
        cb_(r); // 同步消费；若需异步请拷贝 Record 再投递
        }
    void flush_() override {}

private:
    SinkFactory::UiCallback cb_;
    };
} // namespace

std::shared_ptr<spdlog::logger> SinkFactory::create(const LogSinkSpec &spec,
                                                    UiCallback ui_cb)
    {
    std::vector<spdlog::sink_ptr> sinks;
    auto level_of = [&](const std::string &s)
        {
        return static_cast<spdlog::level::level_enum>(level_from_name(s));
        };

    if (spec.file_enabled)
        {
        auto s = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            spec.file_path,
            static_cast<std::size_t>(spec.file_max_size_mb) * 1024 * 1024,
            static_cast<std::size_t>(spec.file_rotate_count));
        s->set_level(level_of(spec.file_level));
        sinks.push_back(std::move(s));
        }
    if (spec.ui_enabled && ui_cb)
        {
        auto s = std::make_shared<UiSink>(std::move(ui_cb));
        s->set_level(level_of(spec.ui_level));
        sinks.push_back(std::move(s));
        }

    auto logger =
        std::make_shared<spdlog::logger>("app", sinks.begin(), sinks.end());
    // 总闸放最松，是否输出由各 sink 自己的级别过滤
    logger->set_level(spdlog::level::trace);
    logger->set_pattern(spec.pattern);
    logger->flush_on(spdlog::level::warn);
    return logger;
    }

} // namespace app::adapters
