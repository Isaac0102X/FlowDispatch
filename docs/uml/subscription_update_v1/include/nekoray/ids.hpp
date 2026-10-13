#pragma once

#include <compare>
#include <functional>
#include <string>
#include <utility>

namespace nekoray {

struct NodeId {
    std::string value;
    auto operator<=>(const NodeId&) const = default;
};

struct SubscriptionId {
    std::string value;
    auto operator<=>(const SubscriptionId&) const = default;
};

} // namespace nekoray

namespace std {
template <> struct hash<nekoray::NodeId> {
    size_t operator()(const nekoray::NodeId& id) const noexcept {
        return hash<string>{}(id.value);
    }
};
template <> struct hash<nekoray::SubscriptionId> {
    size_t operator()(const nekoray::SubscriptionId& id) const noexcept {
        return hash<string>{}(id.value);
    }
};
} // namespace std
