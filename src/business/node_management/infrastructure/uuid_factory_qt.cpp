#include "uuid_factory.h"

#include <QUuid>

namespace nekoray{

namespace {

std::string createUuidString()
    {
    return QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    }

    } // namespace

NodeId CreateNodeId() { return NodeId{createUuidString()}; }
SubscriptionId CreateSubscriptionId() { return SubscriptionId{createUuidString()}; }

} // namespace nekoray
