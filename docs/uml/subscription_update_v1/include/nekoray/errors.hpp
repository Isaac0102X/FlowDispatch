#pragma once

#include <stdexcept>
#include <string>
#include <utility>

namespace nekoray {

class SubscriptionUpdateError : public std::runtime_error {
public:
    //继承构造函数（inheriting constructors）,C++11
    using std::runtime_error::runtime_error;
};

class SubscriptionNotFoundError final : public SubscriptionUpdateError {
public:
    explicit SubscriptionNotFoundError(const std::string& id)
        : SubscriptionUpdateError("Subscription not found: " + id) {}
};

// One type covers direct/proxy transport failures; the message records the concrete cause.
class SubscriptionFetchError final : public SubscriptionUpdateError {
public:
    using SubscriptionUpdateError::SubscriptionUpdateError;
};

class SubscriptionParseError : public SubscriptionUpdateError {
public:
    using SubscriptionUpdateError::SubscriptionUpdateError;
};

class NoValidNodesError final : public SubscriptionParseError {
public:
    NoValidNodesError() : SubscriptionParseError("Subscription contained no valid nodes") {}
};

class RepositoryError final : public SubscriptionUpdateError {
public:
    using SubscriptionUpdateError::SubscriptionUpdateError;
};

} // namespace nekoray
