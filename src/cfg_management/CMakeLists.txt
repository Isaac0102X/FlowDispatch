add_library(app_cfg_management STATIC
    ConfigRegistry.cpp
    BasicSettings/LogConfigSection.cpp
    BasicSettings/CommonSection.cpp
)

target_link_libraries(app_cfg_management PUBLIC app_infra)
