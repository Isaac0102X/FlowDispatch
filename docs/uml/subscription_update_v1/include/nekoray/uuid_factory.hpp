#pragma once

#include "nekoray/ids.hpp"

namespace nekoray {

// Infrastructure helpers: UUID generation stays outside Domain logic.
NodeId CreateNodeId();
SubscriptionId CreateSubscriptionId();

} // namespace nekoray
