#include "nekoray/node.hpp"

#include <algorithm>
#include <cctype>
#include <string_view>

namespace nekoray {
namespace {

const nlohmann::json& configObject(const nlohmann::json& fields) {
    // Some NekoRay-style records wrap protocol fields inside "bean".
    if (fields.is_object()) {
        const auto it = fields.find("bean");
        if (it != fields.end() && it->is_object()) {
            return *it;
        }
    }
    return fields;
}

std::string stringField(const nlohmann::json& object, std::string_view name) {
    const auto it = object.find(std::string{name});
    if (it == object.end() || !it->is_string()) {
        return {};
    }
    return it->get<std::string>();
}

std::string extractCredential(const std::string& protocol, const nlohmann::json& rawFields) {
    const auto& fields = configObject(rawFields);

    // V1 matching heuristic. Protocol schemas may need refinement as parsers are implemented.
    if (protocol == "vmess" || protocol == "vless") {
        auto value = stringField(fields, "uuid");
        if (value.empty()) value = stringField(fields, "id");
        return value;
    }
    if (protocol == "ss" || protocol == "shadowsocks" ||
        protocol == "trojan" || protocol == "hysteria2") {
        return stringField(fields, "password");
    }

    // Unknown protocol: do not guess between fields with different semantics.
    return {};
}

} // namespace

Node::Node(NodeId id,
           std::optional<SubscriptionId> subscriptionId,
           std::string protocol,
           std::string address,
           std::uint16_t port,
           ProtocolConfig protocolConfig)
    : id_(std::move(id)),
      subscriptionId_(std::move(subscriptionId)),
      protocol_(std::move(protocol)),
      address_(std::move(address)),
      port_(port),
      protocolConfig_(std::move(protocolConfig)) {}

NodeIdentityKey Node::identityKey() const {
    return NodeIdentityKey{
        protocol_, address_, port_, extractCredential(protocol_, protocolConfig_.fields)};
}

Node Node::AssignId(NodeId id) const {
    Node copy = *this;
    copy.id_ = std::move(id);
    return copy;
}

} // namespace nekoray
