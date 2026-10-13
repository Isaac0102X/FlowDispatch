#pragma once

#include "nekoray/ids.hpp"
#include "nekoray/protocol_config.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace nekoray {

struct NodeIdentityKey {
    std::string protocol;
    std::string address;
    std::uint16_t port{};
    std::string credential;
    auto operator<=>(const NodeIdentityKey&) const = default;
};

class Node {
public:
    Node(NodeId id,
         std::optional<SubscriptionId> subscriptionId,
         std::string protocol,
         std::string address,
         std::uint16_t port,
         ProtocolConfig protocolConfig);

    const NodeId& id() const noexcept { return id_; }
    const std::optional<SubscriptionId>& subscriptionId() const noexcept { return subscriptionId_; }
    const std::string& protocol() const noexcept { return protocol_; }
    const std::string& address() const noexcept { return address_; }
    std::uint16_t port() const noexcept { return port_; }
    const ProtocolConfig& protocolConfig() const noexcept { return protocolConfig_; }

    NodeIdentityKey identityKey() const;

    // Returns a value copy with a replacement persistent ID; does not mutate this object.
    Node AssignId(NodeId id) const;
    void detachFromSubscription() noexcept { subscriptionId_.reset(); }

private:
    NodeId id_;
    std::optional<SubscriptionId> subscriptionId_;
    std::string protocol_;
    std::string address_;
    std::uint16_t port_{};
    ProtocolConfig protocolConfig_;
};

} // namespace nekoray
