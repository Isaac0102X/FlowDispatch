#include "nekoray/uuid_factory.hpp"

#include <QUuid>

namespace nekoray {
namespace {
std::string createUuidString() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
}
} // namespace

NodeId CreateNodeId() { return NodeId{createUuidString()}; }
SubscriptionId CreateSubscriptionId() { return SubscriptionId{createUuidString()}; }

} // namespace nekoray
