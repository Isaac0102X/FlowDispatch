add_subdirectory(infra)
add_subdirectory(core)
add_subdirectory(business)
add_subdirectory(adapters)
add_subdirectory(ui)
add_subdirectory(bootstrap)


add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE app_bootstrap Qt6::Widgets)
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}>"
)


# 需要拷贝到bin的文件
set(ASSETS
    ${CMAKE_CURRENT_SOURCE_DIR}/../assets/geoip.dat
    ${CMAKE_CURRENT_SOURCE_DIR}/../assets/geoip.db
    ${CMAKE_CURRENT_SOURCE_DIR}/../assets/geosite.dat
    ${CMAKE_CURRENT_SOURCE_DIR}/../assets/geosite.db
    ${CMAKE_CURRENT_SOURCE_DIR}/../assets/nekobox_core
    ${CMAKE_CURRENT_SOURCE_DIR}/../assets/qtbase_zh_CN.qm
)

# 注册一个构建后事件，自动复制文件，内容不同才覆盖
foreach(ASSET ${ASSETS})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${ASSET}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>   # 这是一个生成器表达式，自动解析为 ${PROJECT_NAME} 所在的绝对路径
        COMMENT "Copying ${ASSET} to executable directory"
    )
endforeach()

# 构建成功后自动创建nodes,groups和dashboard文件夹
# 目录不存在就创建（含缺失的父目录），已存在就什么都不做
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/nodes
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/groups
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/dashboard
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/ui/res/dashboard-notice.html
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/dashboard/index.html
    COMMENT "Creating nodes/groups directory next to ${PROJECT_NAME} executable"
)






