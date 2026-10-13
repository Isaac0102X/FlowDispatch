#pragma once

#include "nekoray/ids.hpp"
#include "nekoray/node.hpp"
#include "nekoray/subscription.hpp"

#include <optional>
#include <string>
#include <vector>

namespace nekoray {

enum class FetchMode { Direct, Proxy };
enum class UpdateTrigger { Manual, Scheduled };

class SubscriptionRepository {
public:
    virtual ~SubscriptionRepository() = default;
    virtual Subscription* findById(const SubscriptionId& id) = 0;
    virtual void save(const Subscription& subscription) = 0;
};

class SubscriptionFetcher {
public:
    virtual ~SubscriptionFetcher() = default;
    virtual std::string fetch(const Subscription& subscription, FetchMode mode) = 0;
};

class NodeLinkParser {
public:
    virtual ~NodeLinkParser() = default;
    // Returns successfully parsed nodes. Throw SubscriptionParseError for an unusable payload.
    virtual std::vector<Node> parse(const std::string& rawData,
                                    const SubscriptionId& sourceSubscription) = 0;
};

class ProxyCore {
public:
    virtual ~ProxyCore() = default;
    virtual std::optional<NodeId> activeNodeId() const = 0;
};

class NodeRepository {
public:
    virtual ~NodeRepository() = default;
    virtual const Node* findById(const NodeId& id) const = 0;
    virtual void replaceSubscriptionNodes(const SubscriptionId& subscriptionId,
                                          std::vector<Node> newNodes,
                                          std::optional<NodeId> protectedNodeId) = 0;
};

class UpdateLogger {
public:
    virtual ~UpdateLogger() = default;
    virtual void error(const std::string& message) = 0;
};

class UpdateNotifier {
public:
    virtual ~UpdateNotifier() = default;
    virtual void showManualUpdateFailure(const std::string& message) = 0;
    virtual void refreshNodeList() = 0;
};

} // namespace nekoray
