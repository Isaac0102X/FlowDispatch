add_library(app_business STATIC
    node_management/NodeConfigSection.cpp
    node_management/application/node_controller.cpp
    node_management/domain/node.cpp
    node_management/infrastructure/uuid_factory_qt.cpp
    scheduled_tasks/ScheduledTasksController.cpp
    scheduled_tasks/TaskConfigSection.cpp
)

target_link_libraries(app_business PUBLIC app_infra)

target_link_libraries(app_business PUBLIC Qt6::Core nlohmann_json::nlohmann_json)
