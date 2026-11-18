#pragma once

#include "../infrastructure/subscription_update_ports.h"

namespace app::business::node_management::application {

using namespace app::business::node_management::infrastructure;

class NodeController {
public:
    NodeController(SubscriptionRepository& subscriptions,
                   SubscriptionFetcher& fetcher,
                   NodeLinkParser& parser,
                   ProxyCore& core,
                   NodeRepository& nodes,
                   UpdateLogger& logger,
                   UpdateNotifier& notifier)
        : subscriptions_(subscriptions), fetcher_(fetcher), parser_(parser), core_(core),
          nodes_(nodes), logger_(logger), notifier_(notifier) {}

    // Returns true only when the whole update flow reports success.
    bool updateSubscription(const SubscriptionId& id, FetchMode mode, UpdateTrigger trigger);

private:
    SubscriptionRepository& subscriptions_;
    SubscriptionFetcher& fetcher_;
    NodeLinkParser& parser_;
    ProxyCore& core_;
    NodeRepository& nodes_;
    UpdateLogger& logger_;
    UpdateNotifier& notifier_;
};

} // namespace app::business::node_management::infrastructure
