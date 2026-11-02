add_library(app_business STATIC
    node_management/NodeConfigSection.cpp
    scheduled_tasks/ScheduledTasksController.cpp
    scheduled_tasks/TaskConfigSection.cpp
)

target_link_libraries(app_business PUBLIC app_infra)
