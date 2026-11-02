add_library(app_adapter_logging STATIC
    SinkFactory.cpp
    ../spdlog/SpdlogLogger.cpp
)

target_link_libraries(app_adapter_logging
    PUBLIC app_infra
    PRIVATE spdlog::spdlog_header_only
)
