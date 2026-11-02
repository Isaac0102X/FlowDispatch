add_library(app_core STATIC
    AppSettingsSection.cpp
    ConfigRegistry.cpp
    LogConfigSection.cpp
)

target_link_libraries(app_core PUBLIC app_infra)
