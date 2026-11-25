add_subdirectory(infra)
add_subdirectory(cfg_management)
add_subdirectory(business)
add_subdirectory(adapters)
add_subdirectory(ui)
add_subdirectory(bootstrap)


add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE app_bootstrap Qt6::Widgets)
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}>"
)

# ---------- 资产列表 ----------
set(ASSET_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../assets/geoip.dat
    ${CMAKE_CURRENT_SOURCE_DIR}/../assets/geoip.db
    ${CMAKE_CURRENT_SOURCE_DIR}/../assets/geosite.dat
    ${CMAKE_CURRENT_SOURCE_DIR}/../assets/geosite.db
    ${CMAKE_CURRENT_SOURCE_DIR}/../assets/qtbase_zh_CN.qm
)

set(ASSET_PROGRAMS
    ${CMAKE_CURRENT_SOURCE_DIR}/../assets/nekobox_core
)

# 开发构建时统一遍历，不区分权限
set(ASSETS ${ASSET_FILES} ${ASSET_PROGRAMS})

# ---------- 开发构建：POST_BUILD 保持原样 ----------
foreach(ASSET ${ASSETS})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${ASSET}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying ${ASSET} to executable directory"
    )
endforeach()

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
    COMMENT "Creating nodes/groups/dashboard next to ${PROJECT_NAME} executable"
)

# ---------- 出包安装：追加到已有 install(TARGETS ...) 后面 ----------

# 1) 普通资产文件，复制到 dist 根目录
install(FILES ${ASSET_FILES}
        DESTINATION .)

# 2) 可执行资产，保留可执行权限；Windows 上效果同 FILES，跨平台更稳
install(PROGRAMS ${ASSET_PROGRAMS}
        DESTINATION .)

# 3) dashboard/index.html，顺便自动创建 dashboard 目录
install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/res/dashboard-notice.html
        DESTINATION dashboard
        RENAME index.html)

# 4) 空目录 nodes/groups：install 没有“只建空目录”的语法，用 install(CODE)
install(CODE [[
    file(MAKE_DIRECTORY "${CMAKE_INSTALL_PREFIX}/nodes")
    file(MAKE_DIRECTORY "${CMAKE_INSTALL_PREFIX}/groups")
]])

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
#install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/README.md" DESTINATION . OPTIONAL)

# 模板清单：新增业务配置段时，在这里加一行即可（其余构建逻辑不用改）。
set(APP_CONFIG_TEMPLATES
    app.json
    basic/log.json
    basic/common.json
    )

foreach(tpl IN LISTS APP_CONFIG_TEMPLATES)
    get_filename_component(tpl_dir  "${tpl}" DIRECTORY)   # node_management / 空
    get_filename_component(tpl_name "${tpl}" NAME_WE)     # node_config / app
    # 模板在发布时统一改名为 *.default.json，与用户配置 *.json 明确区分
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../assets/config/${tpl}"
            DESTINATION "Config/${tpl_dir}"
            RENAME "${tpl_name}.default.json")
endforeach()

# 一条命令生成绿色发布目录（方便手工出包 / 打 zip）：
#   cmake --build . --target package_portable
add_custom_target(package_portable
    COMMAND ${CMAKE_COMMAND} --install "${CMAKE_BINARY_DIR}"
            --prefix "${CMAKE_BINARY_DIR}/dist" --config $<CONFIG>
    DEPENDS ${PROJECT_NAME}
    COMMENT "生成绿色发布目录: ${CMAKE_BINARY_DIR}/dist (exe + Config/*.default.json，不含用户配置)"
    VERBATIM)

# Windows 绿色包还需要 Qt 运行库（DLL）；能找到 windeployqt 时提供一条部署命令：
#   cmake --build . --target deploy_qt_runtime
if(WIN32)
    find_program(WINDEPLOYQT_EXE NAMES windeployqt HINTS "${Qt6_DIR}/../../../bin")
    if(NOT WINDEPLOYQT_EXE)
        find_program(WINDEPLOYQT_EXE NAMES windeployqt)
    endif()
    if(WINDEPLOYQT_EXE)
        add_custom_target(deploy_qt_runtime
            COMMAND "${WINDEPLOYQT_EXE}" --no-translations "$<TARGET_FILE:doubao_qt_app>"
            COMMENT "windeployqt: 把 Qt 运行库拷贝到 exe 旁边（绿色版发布前执行）"
            VERBATIM)
    else()
        message(STATUS "未找到 windeployqt，Windows 发布请手动用 Qt 自带的 windeployqt 部署 DLL")
    endif()
endif()

