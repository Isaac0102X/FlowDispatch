#pragma once

#include <compare>
#include <functional>
#include <string>
#include <utility>

/*
 * strong types（强类型包装）
this design is to let the compiler prevent you from mixing up NodeId/SubscriptionId if they are both represented as strings.
for example, you could pass a SubscriptionId to a function that expects a NodeId, and the compiler will catch it.
*/

struct NodeId
    {
    std::string value;
    auto operator<=>(const NodeId &) const = default;
    };

struct SubscriptionId
    {
    std::string value;
    auto operator<=>(const SubscriptionId &) const = default;
    };

namespace std{

template <> struct hash<NodeId>
    {
    size_t operator()(const NodeId &id) const noexcept
        {
        return hash<string>{}(id.value);
        }
    };
template <> struct hash<SubscriptionId>
    {
    size_t operator()(const SubscriptionId &id) const noexcept
        {
        return hash<string>{}(id.value);
        }
    };

} // namespace std
