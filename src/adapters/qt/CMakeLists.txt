add_library(app_adapter_qt STATIC
    QtLogBridge.cpp
    QtLogBridge.h
)

set_target_properties(app_adapter_qt PROPERTIES AUTOMOC ON)
target_link_libraries(app_adapter_qt PUBLIC app_infra Qt6::Core)
