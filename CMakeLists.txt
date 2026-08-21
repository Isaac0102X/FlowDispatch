cmake_minimum_required(VERSION 3.16)

project(FlowDispatch VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

#### Platform Variables ####
#避免主 CMakeLists.txt 充斥大量 if(WIN32)...elseif(UNIX)... 嵌套
if (WIN32)
    include("cmake/windows/windows.cmake")
else ()
    include("cmake/linux/linux.cmake")
endif ()

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
        ui/ThemeManager.cpp
        ui/Icon.cpp
        ui/mainwindow.cpp
        ui/mainwindow.h
        ui/mainwindow.ui
        main/main.cpp

        # ---- 编辑对话框 ----
        # ui/edit/dialog_edit_profile.h
        # ui/edit/dialog_edit_profile.cpp
        # ui/edit/dialog_edit_profile.ui
        # ui/edit/dialog_edit_group.h
        # ui/edit/dialog_edit_group.cpp
        # ui/edit/dialog_edit_group.ui

        # ui/edit/edit_chain.h
        # ui/edit/edit_chain.cpp
        # ui/edit/edit_chain.ui
        # ui/edit/edit_socks_http.h
        # ui/edit/edit_socks_http.cpp
        # ui/edit/edit_socks_http.ui
        # ui/edit/edit_shadowsocks.h
        # ui/edit/edit_shadowsocks.cpp
        # ui/edit/edit_shadowsocks.ui
        # ui/edit/edit_vmess.h
        # ui/edit/edit_vmess.cpp
        # ui/edit/edit_vmess.ui
        # ui/edit/edit_trojan_vless.h
        # ui/edit/edit_trojan_vless.cpp
        # ui/edit/edit_trojan_vless.ui
        # ui/edit/edit_naive.h
        # ui/edit/edit_naive.cpp
        # ui/edit/edit_naive.ui
        # ui/edit/edit_quic.h
        # ui/edit/edit_quic.cpp
        # ui/edit/edit_quic.ui
        # ui/edit/edit_custom.h
        # ui/edit/edit_custom.cpp
        # ui/edit/edit_custom.ui
        # TODO: 这些 edit_* 对话框内部会引用 AbstractBean 派生类（ShadowSocksBean、VMessBean 等），
        # 属于 fmt/ 下的数据结构；重写业务逻辑时要么保留这套 Bean 结构，要么把它们
        # 改造成调用你自己的配置数据模型的 getter/setter

        # ---- 其它设置对话框 ----
        ui/dialog_basic_settings.cpp
        ui/dialog_basic_settings.h
        ui/dialog_basic_settings.ui
        ui/dialog_manage_groups.cpp
        ui/dialog_manage_groups.h
        ui/dialog_manage_groups.ui
        # ui/dialog_manage_routes.cpp
        # ui/dialog_manage_routes.h
        # ui/dialog_manage_routes.ui
        ui/dialog_vpn_settings.cpp
        ui/dialog_vpn_settings.h
        ui/dialog_vpn_settings.ui
        # ui/dialog_hotkey.cpp
        # ui/dialog_hotkey.h
        # ui/dialog_hotkey.ui
        # TODO: dialog_hotkey 依赖 QHotkey 库，不需要全局热键的话可以整组删掉

        # ---- 列表项 widget ----
        ui/widget/ProxyItem.cpp
        ui/widget/ProxyItem.h
        ui/widget/ProxyItem.ui
        ui/widget/GroupItem.cpp
        ui/widget/GroupItem.h
        ui/widget/GroupItem.ui

        # ---- 资源文件 ----
        res/neko.qrc
        res/theme/feiyangqingyun/qss.qrc
        # TODO: 图标/qss 资源建议改名或替换成你自己的品牌资源
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

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
