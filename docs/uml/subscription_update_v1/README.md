# Subscription Update V1 — C++20 skeleton

This is an implementation skeleton for the current Subscription Update scope, not a complete application. Network fetching, protocol-link parsing, JSON file persistence, logging/UI integration, and repository mutation are ports to be implemented by concrete infrastructure classes.

## Decisions reflected here

- `NodeId` and `SubscriptionId` are strong wrappers around UUID strings.
- UUID generation uses `QUuid::createUuid()` in Infrastructure helpers.
- `ProtocolConfig` uses `nlohmann::json`; keep configuration JSON on this representation at boundaries too, and convert only where a Qt API requires it.
- `NodeIdentityKey` is a V1 heuristic: protocol + address + port + protocol credential. Current extraction supports common `uuid`/`id` fields for VMess/VLESS and `password` for SS/Trojan/Hysteria2. Validate/adjust field names against the actual parser schemas before relying on identity preservation.
- `Node::AssignId()` returns a value copy with the supplied persistent ID. The parser creates a new ID for each successfully parsed node; the controller may replace the ID of the active-node match.
- `updateSubscription()` fetches and parses before repository mutation; an empty parsed result is rejected so old subscription nodes are not cleared by an empty response.
- Exceptions are caught at the controller boundary. Manual updates show an error; scheduled updates log silently. No transaction/rollback guarantee is implemented.

## Build requirements

- C++20 compiler
- Qt 6 Core
- `nlohmann_json` CMake package

```sh
cmake -S . -B build
cmake --build build
```

The target is a library of interfaces and controller logic; no executable/test target is included yet.

## Important V1 limitations

1. `NodeRepository::replaceSubscriptionNodes()` must validate that incoming nodes belong to the target subscription, IDs are unique, and any protected node belongs to that subscription.
2. Persistence failure during `replaceSubscriptionNodes()` or `SubscriptionRepository::save()` may leave partially updated disk state. V1 reports failure; rollback/recovery is V2 TODO.
3. `NodeIdentityKey` currently compares strings exactly. Protocol-specific canonicalization and edge cases remain to be verified.
4. `NodeController` treats any empty parser result as failure. Whether a payload with both valid and invalid entries should partially succeed is not specified here; decide in the concrete parser based on the finalized UC/contract.
5. Fetch mode is an explicit `Direct` or `Proxy` input in the current snapshot; this skeleton does not silently retry through the other mode.
