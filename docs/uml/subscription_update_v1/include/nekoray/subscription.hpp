#pragma once

#include "nekoray/ids.hpp"

#include <chrono>
#include <string>
#include <utility>

namespace nekoray {

using DateTime = std::chrono::system_clock::time_point;

class Subscription {
public:
    Subscription(SubscriptionId id, std::string name, std::string url)
        : id_(std::move(id)), name_(std::move(name)), url_(std::move(url)) {}

    const SubscriptionId& id() const noexcept { return id_; }
    const std::string& name() const noexcept { return name_; }
    const std::string& url() const noexcept { return url_; }
    DateTime lastUpdatedAt() const noexcept { return lastUpdatedAt_; }
    void markUpdated(DateTime at) noexcept { lastUpdatedAt_ = at; }

private:
    SubscriptionId id_;
    std::string name_;
    std::string url_;
    DateTime lastUpdatedAt_{};
};

} // namespace nekoray
