#pragma once

#include "../domain/ids.h"

namespace nekoray {

// Infrastructure helpers: UUID generation stays outside Domain logic.
NodeId CreateNodeId();
SubscriptionId CreateSubscriptionId();

} // namespace nekoray
