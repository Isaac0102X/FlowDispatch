cmake_minimum_required(VERSION 3.16)

project(FrontEnd VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 检测当前项目是否被作为子目录包含
# 如果 CMAKE_SOURCE_DIR（顶层项目路径）等于 CMAKE_CURRENT_SOURCE_DIR（当前路径），说明是独立构建
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # 独立构建时，设置输出路径为当前构建目录下的 bin
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    message(STATUS "${PROJECT_NAME} is building as a standalone project, output to ${CMAKE_BINARY_DIR}/bin")
else()
    # 被父级包含时，不做任何事，完全继承父级的设置
    message(STATUS "${PROJECT_NAME} is building as subdirectory, inheriting output paths from root")
endif()

# WINDOWS PDB FILE
if (MSVC)
	#/Zi：告诉 MSVC 编译器生成完整的调试信息，并将其存储在独立的 .pdb 文件中
	add_compile_options("$<$<CONFIG:Release>:/Zi>")
	#单独使用 /DEBUG 会禁用链接器的优化（REF 和 ICF），导致 Release 产物体积显著增大。显式加上 /OPT:REF /OPT:ICF 就是为了在保留调试符号的同时不牺牲体积优化
	add_link_options("$<$<CONFIG:Release>:/DEBUG;/OPT:REF;/OPT:ICF>")
endif ()

# Find Qt
if (NOT QT_VERSION_MAJOR)
    set(QT_VERSION_MAJOR 6)
endif ()
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Svg LinguistTools)

# Platform Variables ####
#避免主 CMakeLists.txt 充斥大量 if(WIN32)...elseif(UNIX)... 嵌套
#if (WIN32)
#    include("cmake/windows/windows.cmake")
#else ()
#    include("cmake/linux/linux.cmake")
#endif ()

#多线程通常需要链接 -pthread 标志，它自动处理这些细节，避免手动指定编译选项
find_package(Threads)

# yaml-cpp
# if (NKR_NO_YAML)
#     nkr_add_compile_definitions(NKR_NO_YAML)
# else ()
#     find_package(yaml-cpp CONFIG REQUIRED) # only Release is built
#     list(APPEND NKR_EXTERNAL_TARGETS yaml-cpp)
# endif ()

set(PROJECT_SOURCES
        # ---- UI 通用小控件（3rdparty，跟业务逻辑无耦合，直接复用）----
        # 若不需要 JSON 编辑器/自动补全框，可以去掉
        # 3rdparty/QtExtKeySequenceEdit.cpp
        # 3rdparty/qv2ray/v2/ui/QvAutoCompleteTextEdit.cpp
        # 3rdparty/qv2ray/v2/ui/widgets/common/QJsonModel.cpp
        # 3rdparty/qv2ray/v2/ui/widgets/editors/w_JsonEditor.cpp
        # 3rdparty/qv2ray/v2/ui/widgets/editors/w_JsonEditor.hpp
        # 3rdparty/qv2ray/v2/ui/widgets/editors/w_JsonEditor.ui

        # ---- 主窗口 ----
        Frontend/ThemeManager.cpp
        Frontend/Icon.cpp
        Frontend/mainwindow.cpp
        Frontend/mainwindow.h
        Frontend/mainwindow.ui
        Frontend/main.cpp

        # ---- 编辑对话框 ----
        # Frontend/edit/dialog_edit_profile.h
        # Frontend/edit/dialog_edit_profile.cpp
        # Frontend/edit/dialog_edit_profile.ui
        # Frontend/edit/dialog_edit_group.h
        # Frontend/edit/dialog_edit_group.cpp
        # Frontend/edit/dialog_edit_group.ui

        # Frontend/edit/edit_chain.h
        # Frontend/edit/edit_chain.cpp
        # Frontend/edit/edit_chain.ui
        # Frontend/edit/edit_socks_http.h
        # Frontend/edit/edit_socks_http.cpp
        # Frontend/edit/edit_socks_http.ui
        # Frontend/edit/edit_shadowsocks.h
        # Frontend/edit/edit_shadowsocks.cpp
        # Frontend/edit/edit_shadowsocks.ui
        # Frontend/edit/edit_vmess.h
        # Frontend/edit/edit_vmess.cpp
        # Frontend/edit/edit_vmess.ui
        # Frontend/edit/edit_trojan_vless.h
        # Frontend/edit/edit_trojan_vless.cpp
        # Frontend/edit/edit_trojan_vless.ui
        # Frontend/edit/edit_naive.h
        # Frontend/edit/edit_naive.cpp
        # Frontend/edit/edit_naive.ui
        # Frontend/edit/edit_quic.h
        # Frontend/edit/edit_quic.cpp
        # Frontend/edit/edit_quic.ui
        # Frontend/edit/edit_custom.h
        # Frontend/edit/edit_custom.cpp
        # Frontend/edit/edit_custom.ui
        # TODO: 这些 edit_* 对话框内部会引用 AbstractBean 派生类（ShadowSocksBean、VMessBean 等），
        # 属于 fmt/ 下的数据结构；重写业务逻辑时要么保留这套 Bean 结构，要么把它们
        # 改造成调用你自己的配置数据模型的 getter/setter

        # ---- 其它设置对话框 ----
        Frontend/dialog_basic_settings.cpp
        Frontend/dialog_basic_settings.h
        Frontend/dialog_basic_settings.ui
        Frontend/dialog_manage_groups.cpp
        Frontend/dialog_manage_groups.h
        Frontend/dialog_manage_groups.ui
        # Frontend/dialog_manage_routes.cpp
        # Frontend/dialog_manage_routes.h
        # Frontend/dialog_manage_routes.ui
        Frontend/dialog_vpn_settings.cpp
        Frontend/dialog_vpn_settings.h
        Frontend/dialog_vpn_settings.ui
        # Frontend/dialog_hotkey.cpp
        # Frontend/dialog_hotkey.h
        # Frontend/dialog_hotkey.ui
        # TODO: dialog_hotkey 依赖 QHotkey 库，不需要全局热键的话可以整组删掉

        # ---- 列表项 widget ----
        Frontend/widget/ProxyItem.cpp
        Frontend/widget/ProxyItem.h
        Frontend/widget/ProxyItem.ui
        Frontend/widget/GroupItem.cpp
        Frontend/widget/GroupItem.h
        Frontend/widget/GroupItem.ui

        # ---- 资源文件 ----
        res/neko.qrc
        res/theme/feiyangqingyun/qss.qrc
        # TODO: 图标/qss 资源建议改名或替换成你自己的品牌资源
)

# 递归搜索 ../../Include 下所有的 .h 和 .hpp 文件
file(GLOB_RECURSE INCLUDED_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Include/*.hpp
)

# 用 list(APPEND) 把收集到的头文件追加上去
list(APPEND PROJECT_SOURCES ${INCLUDED_HEADERS})

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

#附加包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Include
)

# 需要拷贝到bin的文件
set(ASSETS
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/geoip.dat
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/geoip.db
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/geosite.dat
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/geosite.db
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/nekobox_core
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/qtbase_zh_CN.qm
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/FlowDispatch.json
)

# 关键：注册一个构建后事件，自动复制文件
foreach(ASSET ${ASSETS})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${ASSET}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>   # 这是一个生成器表达式，自动解析为 ${PROJECT_NAME} 所在的绝对路径
        COMMENT "Copying ${ASSET} to executable directory"
    )
endforeach()

# 构建成功后自动创建groups和dashboard文件夹
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/groups
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/dashboard
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/res/dashboard-notice.html
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/dashboard/index.html
    COMMENT "Creating groups directory next to ${PROJECT_NAME} executable"
)

#CMake 中用于启用 Qt 自动化工具 的配置命令
set_property(TARGET ${PROJECT_NAME} PROPERTY AUTOUIC ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY AUTOMOC ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY AUTORCC ON)

# Windows 下不弹控制台窗口，如果需要看日志可以改成 FALSE 或删除
set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
)

# 翻译文件（可选）
# TODO: 如果沿用原来的翻译文本可以保留，不需要国际化就整段删掉
# set(TS_FILES
#         translations/zh_CN.ts
# )
# set(LUPDATE_OPTIONS -locations none -no-obsolete)
# if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
#     qt_add_lupdate(yourapp TS_FILES ${TS_FILES} OPTIONS ${LUPDATE_OPTIONS})
#     qt_add_lrelease(yourapp TS_FILES ${TS_FILES} QM_FILES_OUTPUT_VARIABLE QM_FILES)
# else ()
#     qt5_create_translation(QM_FILES ${PROJECT_SOURCES} ${TS_FILES} OPTIONS ${LUPDATE_OPTIONS})
# endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Svg
        Threads::Threads
        # ${NKR_EXTERNAL_TARGETS}   # 如果保留了 grpc/yaml-cpp/zxing/qhotkey 等依赖，取消注释
        ${PLATFORM_LIBRARIES}     #平台 cmake 文件
        # TODO: 加上你业务逻辑需要的库，比如 sqlite、json 解析库等
)

if (QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif ()
