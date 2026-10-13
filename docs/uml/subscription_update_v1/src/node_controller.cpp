#include "nekoray/node_controller.hpp"
#include "nekoray/errors.hpp"

#include <algorithm>
#include <chrono>
#include <exception>
#include <string>
#include <utility>
#include <vector>

namespace nekoray {
namespace {

std::string messageFor(const std::exception& e) {
    return e.what();
}

} // namespace

bool NodeController::updateSubscription(const SubscriptionId& id,
                                        FetchMode mode,
                                        UpdateTrigger trigger) {
    try {
        Subscription* subscription = subscriptions_.findById(id);
        if (subscription == nullptr) {
            throw SubscriptionNotFoundError(id.value);
        }

        // Fetch and parse before mutating repository state: fetch/parse failures leave old nodes intact.
        const std::string rawData = fetcher_.fetch(*subscription, mode);
        std::vector<Node> newNodes = parser_.parse(rawData, subscription->id());
        if (newNodes.empty()) {
            throw NoValidNodesError{};
        }

        std::optional<NodeId> protectedNodeId;
        if (const auto activeId = core_.activeNodeId()) {
            const Node* oldActive = nodes_.findById(*activeId);
            if (oldActive != nullptr && oldActive->subscriptionId() == subscription->id()) {
                const NodeIdentityKey oldKey = oldActive->identityKey();
                const auto match = std::find_if(newNodes.begin(), newNodes.end(),
                    [&oldKey](const Node& candidate) { return candidate.identityKey() == oldKey; });

                if (match != newNodes.end()) {
                    *match = match->AssignId(oldActive->id());
                } else {
                    protectedNodeId = oldActive->id();
                }
            }
        }

        nodes_.replaceSubscriptionNodes(subscription->id(), std::move(newNodes), protectedNodeId);
        subscription->markUpdated(std::chrono::system_clock::now());
        subscriptions_.save(*subscription);
        notifier_.refreshNodeList();
        return true;
    } catch (const SubscriptionUpdateError& e) {
        logger_.error(std::string{"Subscription update failed: "} + messageFor(e));
        if (trigger == UpdateTrigger::Manual) {
            notifier_.showManualUpdateFailure(e.what());
        }
    } catch (const std::exception& e) {
        logger_.error(std::string{"Unexpected subscription update failure: "} + messageFor(e));
        if (trigger == UpdateTrigger::Manual) {
            notifier_.showManualUpdateFailure("Unexpected error while updating subscription.");
        }
    } catch (...) {
        logger_.error("Unknown non-standard exception while updating subscription.");
        if (trigger == UpdateTrigger::Manual) {
            notifier_.showManualUpdateFailure("Unknown error while updating subscription.");
        }
    }
    return false;
}

} // namespace nekoray
