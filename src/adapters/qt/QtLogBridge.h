// QtLogBridge.h — 把日志从任何线程安全地投递到 UI 线程（经 Qt 信号槽）。
// 适配层。日志层不依赖 Qt；Qt 在这里只充当"投递通道"。
#pragma once

#include "infra/logging/Log.h"

#include <QFileInfo>
#include <QObject>
#include <QString>

#include <functional>

namespace app::adapters::qt{

class QtLogBridge final : public QObject
    {
    Q_OBJECT
    public:
        explicit QtLogBridge(QObject *parent = nullptr) : QObject(parent) {}

        // 给 SinkFactory 用的回调：sink 线程调用，信号槽自动跨线程投递到 UI 线程
        std::function<void(const app::infra::logging::Record &)> callback()
            {
            return [this](const app::infra::logging::Record &r)
                {
                const auto file = QString::fromUtf8(
                    r.file.data(), static_cast<int>(r.file.size()));
                const auto message = QString::fromUtf8(
                    r.message.data(), static_cast<int>(r.message.size()));
                emit recordReceived(QStringLiteral("[%1:%2] %3")
                                        .arg(QFileInfo(file).fileName())
                                        .arg(r.line)
                                        .arg(message));
                };
            }

    signals:
        void recordReceived(const QString &line);
    };

} // namespace app::adapters::qt
