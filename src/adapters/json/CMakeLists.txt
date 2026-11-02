add_library(app_adapter_json STATIC
    JsonConfigStore.cpp
)

target_link_libraries(app_adapter_json
    PUBLIC app_infra
    PRIVATE nlohmann_json::nlohmann_json
)
