#pragma once

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <utility>

namespace nekoray {

// Protocol-specific fields are preserved as JSON. The parser should provide an object.
struct ProtocolConfig {
    nlohmann::json fields = nlohmann::json::object();

    explicit ProtocolConfig(nlohmann::json value = nlohmann::json::object())
        : fields(std::move(value)) {
        if (!fields.is_object()) {
            throw std::invalid_argument("ProtocolConfig.fields must be a JSON object");
        }
    }
};

} // namespace nekoray
