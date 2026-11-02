add_library(app_bootstrap STATIC
    Bootstrapper.cpp
    Bootstrapper.h
)

target_include_directories(app_bootstrap PUBLIC
    "${PROJECT_SOURCE_DIR}/src"
)
target_compile_features(app_bootstrap PUBLIC cxx_std_20)
target_link_libraries(app_bootstrap PRIVATE
    app_adapter_json
    app_adapter_logging
    app_adapter_qt
    app_business
    app_core
    app_infra
    app_ui
    Qt6::Widgets
)
