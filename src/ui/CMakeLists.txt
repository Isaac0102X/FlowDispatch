set(PROJECT_SOURCES
        # ---- 主窗口 ----
        ThemeManager.cpp
        Icon.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui

        # ---- 编辑对话框 ----
        # edit/dialog_edit_profile.h
        # edit/dialog_edit_profile.cpp
        # edit/dialog_edit_profile.ui
        # edit/dialog_edit_group.h
        # edit/dialog_edit_group.cpp
        # edit/dialog_edit_group.ui

        # edit/edit_chain.h
        # edit/edit_chain.cpp
        # edit/edit_chain.ui
        # edit/edit_socks_http.h
        # edit/edit_socks_http.cpp
        # edit/edit_socks_http.ui
        # edit/edit_shadowsocks.h
        # edit/edit_shadowsocks.cpp
        # edit/edit_shadowsocks.ui
        # edit/edit_vmess.h
        # edit/edit_vmess.cpp
        # edit/edit_vmess.ui
        # edit/edit_trojan_vless.h
        # edit/edit_trojan_vless.cpp
        # edit/edit_trojan_vless.ui
        # edit/edit_naive.h
        # edit/edit_naive.cpp
        # edit/edit_naive.ui
        # edit/edit_quic.h
        # edit/edit_quic.cpp
        # edit/edit_quic.ui
        # edit/edit_custom.h
        # edit/edit_custom.cpp
        # edit/edit_custom.ui
        # TODO: 这些 edit_* 对话框内部会引用 AbstractBean 派生类（ShadowSocksBean、VMessBean 等），
        # 属于 fmt/ 下的数据结构；重写业务逻辑时要么保留这套 Bean 结构，要么把它们
        # 改造成调用你自己的配置数据模型的 getter/setter

        # ---- 其它设置对话框 ----
        dialog_basic_settings.cpp
        dialog_basic_settings.h
        dialog_basic_settings.ui
        dialog_manage_groups.cpp
        dialog_manage_groups.h
        dialog_manage_groups.ui
        # dialog_manage_routes.cpp
        # dialog_manage_routes.h
        # dialog_manage_routes.ui
        dialog_vpn_settings.cpp
        dialog_vpn_settings.h
        dialog_vpn_settings.ui
        # dialog_hotkey.cpp
        # dialog_hotkey.h
        # dialog_hotkey.ui
        # TODO: dialog_hotkey 依赖 QHotkey 库，不需要全局热键的话可以整组删掉

        # ---- 列表项 widget ----
        widget/ProxyItem.cpp
        widget/ProxyItem.h
        widget/ProxyItem.ui
        widget/GroupItem.cpp
        widget/GroupItem.h
        widget/GroupItem.ui

        # ---- 资源文件 ----
        res/neko.qrc
        res/theme/feiyangqingyun/qss.qrc
        # TODO: 图标/qss 资源建议改名或替换成你自己的品牌资源
)

add_library(app_ui STATIC ${PROJECT_SOURCES})

target_include_directories(app_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#CMake 中用于启用 Qt 自动化工具 的配置命令
set_property(TARGET app_ui PROPERTY AUTOUIC ON)
set_property(TARGET app_ui PROPERTY AUTOMOC ON)
set_property(TARGET app_ui PROPERTY AUTORCC ON)

target_link_libraries(app_ui PUBLIC
    app_adapter_qt
    app_business
    app_core
    app_infra
    Qt6::Widgets
    Qt6::Network
    Qt6::Svg
)


