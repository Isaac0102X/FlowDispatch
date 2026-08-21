#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "ui/Icon.hpp"
#include "ui/ThemeManager.hpp"
#include "ui/dialog_basic_settings.h"
#include "ui/dialog_hotkey.h"
#include "ui/dialog_manage_groups.h"
#include "ui/dialog_manage_routes.h"
#include "ui/dialog_vpn_settings.h"
#include "ui/edit/dialog_edit_profile.h"


#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QScreen>
#include <QScrollBar>
#include <QTextBlock>
#include <QThread>
#include <QTimer>

inline QWidget *mainwindow;

void UI_InitMainWindow()
    {
    mainwindow = new MainWindow;
    static_cast<MainWindow*>(mainwindow)->show();
    }

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
    {
    mainwindow = this;

    // ---------- UI 初始化（保留） ----------
    //themeManager->ApplyTheme(NekoGui::dataStore->theme); // UI 主题
    ui->setupUi(this);

           // 菜单连接（保留，内部槽函数实现为空）
    connect(ui->menu_start, &QAction::triggered, this, [=]() { /* 业务已注释 */ });
    connect(ui->menu_stop, &QAction::triggered, this, [=]() { /* 业务已注释 */ });

           // 标签页拖动（保留 UI 交互，内部业务逻辑注释）
    connect(ui->tabWidget->tabBar(), &QTabBar::tabMoved, this,
        [=](int from, int to)
    {
    });

    ui->label_running->installEventFilter(this);
    ui->label_inbound->installEventFilter(this);
    ui->splitter->installEventFilter(this);

    // 热键注册（业务，注释）
    //Deletable RegisterHotkey(false);

    // 创建 dashboard 目录（UI 相关，保留，但内容可忽略）
    if (QDir("dashboard").count() == 0)
        {
        QDir().mkdir("dashboard");
        QFile::copy(":/neko/dashboard-notice.html", "dashboard/index.html");
        }

    // 顶部工具栏（UI）
    ui->toolButton_program->setMenu(ui->menu_program);
    ui->toolButton_preferences->setMenu(ui->menu_preferences);
    ui->toolButton_server->setMenu(ui->menu_server);
    ui->menubar->setVisible(false);

    // connect(ui->toolButton_document, &QToolButton::clicked, this, [=]
    //     {
    //     QDesktopServices::openUrl(QUrl("https://matsuridayo.github.io/"));
    //     });
    // connect(ui->toolButton_ads, &QToolButton::clicked, this, [=]
    //     {
    //     QDesktopServices::openUrl(QUrl("https://neko-box.pages.dev/喵"));
    //     });

    // 更新按钮点击（业务，注释）
    //Deletable connect(ui->toolButton_update, &QToolButton::clicked, this, [=] { CheckUpdate(); });
    // 测速按钮点击（业务，注释）
    //Deletable connect(ui->toolButton_url_test, &QToolButton::clicked, this, [=] { speedtest_current_group(1, true); });

   // 日志回调（业务，注释）
   //Deletable MW_show_log = [=](const QString &log) { runOnUiThread([=] { show_log_impl(log); }); };
   //Deletable MW_show_log_ext = ...;

    // 表格 UI（保留）
    ui->proxyListTable->callback_save_order = [=] { /* 业务已注释 */ };
    //ui->proxyListTable->refresh_data = [=](int id) { refresh_proxy_list_impl_refresh_data(id); };
    if (auto button = ui->proxyListTable->findChild<QAbstractButton *>(QString(), Qt::FindDirectChildrenOnly))
        {
        //connect(button, &QAbstractButton::clicked, this, [=] { refresh_proxy_list_impl(-1, {GroupSortMethod::ById}); });
        }
    connect(ui->proxyListTable->horizontalHeader(), &QHeaderView::sectionClicked, this, [=](int logicalIndex)
        {
        // 排序逻辑（业务，注释）
        //Deletable ...
        });
    connect(ui->proxyListTable->horizontalHeader(), &QHeaderView::sectionResized, this,
        [=](int logicalIndex, int oldSize, int newSize)
        {
        // 保存列宽（业务，注释）
        //Deletable ...
        });
    ui->tableWidget_conn->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget_conn->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableWidget_conn->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->proxyListTable->verticalHeader()->setDefaultSectionSize(24);

    // 搜索框（UI）
    ui->search->setVisible(false);
    connect(shortcut_ctrl_f, &QShortcut::activated, this, [=]
        {
        ui->search->setVisible(true);
        ui->search->setFocus();
        });
    connect(shortcut_esc, &QShortcut::activated, this, [=]
        {
        if (ui->search->isVisible())
            {
            ui->search->setText("");
            ui->search->textChanged("");
            ui->search->setVisible(false);
            }
        if (select_mode)
            {
            emit profile_selected(-1);
            select_mode = false;
            //refresh_status();
            }
        });
    connect(ui->search, &QLineEdit::textChanged, this, [=](const QString &text)
        {
        // 搜索过滤（纯 UI）
        if (text.isEmpty())
            {
            for (int i = 0; i < ui->proxyListTable->rowCount(); i++)
                ui->proxyListTable->setRowHidden(i, false);
            }
        else
            {
            QList<QTableWidgetItem *> findItem = ui->proxyListTable->findItems(text, Qt::MatchContains);
            for (int i = 0; i < ui->proxyListTable->rowCount(); i++)
                ui->proxyListTable->setRowHidden(i, true);
            for (auto item : findItem)
                if (item != nullptr)
                    ui->proxyListTable->setRowHidden(item->row(), false);
            }
        });

           // 刷新标签页（UI）
    //this->refresh_groups();

           // 托盘（UI）
    tray = new QSystemTrayIcon(this);
    tray->setIcon(Icon::GetTrayIcon(Icon::NONE));
    tray->setContextMenu(ui->menu_program);
    tray->show();
    // connect(tray, &QSystemTrayIcon::activated, this,
    //     [=](QSystemTrayIcon::ActivationReason reason)
    //     {
    //     if (reason == QSystemTrayIcon::Trigger)
    //         {
    //         // if (this->isVisible())
    //         //     hide();
    //         //else
    //             //ActivateWindow(this);
    //         }
    //     });

           // 菜单项（UI）
    connect(ui->menu_open_config_folder, &QAction::triggered, this, [=]
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::currentPath()));
    });
    ui->menu_program_preference->addActions(ui->menu_preferences->actions());
    connect(ui->menu_add_from_clipboard2, &QAction::triggered, ui->menu_add_from_clipboard, &QAction::trigger);
    // 以下菜单动作的业务逻辑已注释（槽函数为空）
    connect(ui->actionRestart_Proxy, &QAction::triggered, this, [=] { /* 业务已注释 */ });
    connect(ui->actionRestart_Program, &QAction::triggered, this, [=] { /* 业务已注释 */ });
    connect(ui->actionShow_window, &QAction::triggered, this, [=] { tray->activated(QSystemTrayIcon::ActivationReason::Trigger); });

           // 菜单显示前的更新（UI 状态）
    connect(ui->menu_program, &QMenu::aboutToShow, this, [=]()
    {
        // 菜单项状态更新（纯 UI 显示）
        //ui->actionRemember_last_proxy->setChecked(NekoGui::dataStore->remember_enable);
        //ui->actionStart_with_system->setChecked(AutoRun_IsEnabled()); // 系统自启动状态（轻量，可保留）
        //ui->actionAllow_LAN->setChecked(QStringList{"::", "0.0.0.0"}.contains(NekoGui::dataStore->inbound_address));
        // 以下为动态填充菜单（业务，注释）
        //Deletable ...
    });
    // 以下菜单触发（业务，注释）
    //Deletable connect(ui->menuActive_Server, &QMenu::triggered, ...);
    //Deletable connect(ui->menuActive_Routing, &QMenu::triggered, ...);
    connect(ui->actionRemember_last_proxy, &QAction::triggered, this, [=](bool checked)
    {
    });
    connect(ui->actionStart_with_system, &QAction::triggered, this, [=](bool checked)
    {
        //AutoRun_SetEnabled(checked);
    });
    connect(ui->actionAllow_LAN, &QAction::triggered, this, [=](bool checked)
    {
        //NekoGui::dataStore->inbound_address = checked ? "::" : "127.0.0.1";
        //Deletable MW_dialog_message("", "UpdateDataStore");
    });

           // VPN / 系统代理复选框（UI 状态）
    connect(ui->checkBox_VPN, &QCheckBox::clicked, this, [=](bool checked)
    {
      //Deletable neko_set_spmode_vpn(checked);
    });
    connect(ui->checkBox_SystemProxy, &QCheckBox::clicked, this, [=](bool checked)
    {
      //Deletable neko_set_spmode_system_proxy(checked);
    });
    connect(ui->menu_spmode, &QMenu::aboutToShow, this, [=]()
    {
        // ui->menu_spmode_disabled->setChecked(!(NekoGui::dataStore->spmode_system_proxy || NekoGui::dataStore->spmode_vpn));
        // ui->menu_spmode_system_proxy->setChecked(NekoGui::dataStore->spmode_system_proxy);
        // ui->menu_spmode_vpn->setChecked(NekoGui::dataStore->spmode_vpn);
    });
    // 以下菜单触发（业务，注释）
    //Deletable connect(ui->menu_spmode_system_proxy, &QAction::triggered, ...);
    //Deletable connect(ui->menu_spmode_vpn, &QAction::triggered, ...);
    //Deletable connect(ui->menu_spmode_disabled, &QAction::triggered, ...);
    //Deletable connect(ui->menu_qr, &QAction::triggered, ...);
    //Deletable connect(ui->menu_tcp_ping, &QAction::triggered, ...);
    //Deletable connect(ui->menu_url_test, &QAction::triggered, ...);
    //Deletable connect(ui->menu_full_test, &QAction::triggered, ...);
    //Deletable connect(ui->menu_stop_testing, &QAction::triggered, ...);

           // 动态菜单移动（保留 UI 结构）
    auto set_selected_or_group = [=](int mode) { ui->menu_server->setProperty("selected_or_group", mode); };
    auto move_tests_to_menu = [=](bool menuCurrent_Select)
    {
        return [=]
        {
            if (menuCurrent_Select)
                {
                ui->menuCurrent_Select->insertAction(ui->actionfake_4, ui->menu_tcp_ping);
                ui->menuCurrent_Select->insertAction(ui->actionfake_4, ui->menu_url_test);
                ui->menuCurrent_Select->insertAction(ui->actionfake_4, ui->menu_full_test);
                ui->menuCurrent_Select->insertAction(ui->actionfake_4, ui->menu_stop_testing);
                ui->menuCurrent_Select->insertAction(ui->actionfake_4, ui->menu_clear_test_result);
                ui->menuCurrent_Select->insertAction(ui->actionfake_4, ui->menu_resolve_domain);
                }
            else
                {
                ui->menuCurrent_Group->insertAction(ui->actionfake_5, ui->menu_tcp_ping);
                ui->menuCurrent_Group->insertAction(ui->actionfake_5, ui->menu_url_test);
                ui->menuCurrent_Group->insertAction(ui->actionfake_5, ui->menu_full_test);
                ui->menuCurrent_Group->insertAction(ui->actionfake_5, ui->menu_stop_testing);
                ui->menuCurrent_Group->insertAction(ui->actionfake_5, ui->menu_clear_test_result);
                ui->menuCurrent_Group->insertAction(ui->actionfake_5, ui->menu_resolve_domain);
                }
            set_selected_or_group(menuCurrent_Select ? 1 : 0);
        };
    };
    connect(ui->menuCurrent_Select, &QMenu::aboutToShow, this, move_tests_to_menu(true));
    connect(ui->menuCurrent_Group, &QMenu::aboutToShow, this, move_tests_to_menu(false));
    connect(ui->menu_server, &QMenu::aboutToHide, this, [=]
    {
        //setTimeout([=] { set_selected_or_group(2); }, this, 200);
    });
    set_selected_or_group(2);

    connect(ui->menu_share_item, &QMenu::aboutToShow, this, [=]
    {
        // 动态菜单文字（UI）
        QString name;
        //auto selected = get_now_selected_list();
        // if (!selected.isEmpty())
        //     {
        //     auto ent = selected.first();
        //     name = ent->bean->DisplayCoreType();
        //     }
        ui->menu_export_config->setVisible(name == software_core_name);
        ui->menu_export_config->setText(tr("Export %1 config").arg(name));
    });

    //refresh_status();

           // ---------- 以下业务初始化全部注释掉 ----------
           //Deletable NekoGui::dataStore->core_token = GetRandomString(32);
           //Deletable NekoGui::dataStore->core_port = MkPort();
           //Deletable if (NekoGui::dataStore->core_port <= 0) NekoGui::dataStore->core_port = 19810;
           //Deletable auto core_path = QApplication::applicationDirPath() + "/nekobox_core";
           //Deletable QStringList args; ...
           //Deletable runOnUiThread([=] { core_process = new NekoGui_sys::CoreProcess(...); ... }, DS_cores);

           // 系统代理记忆（业务）
           //Deletable if (NekoGui::dataStore->remember_enable || NekoGui::dataStore->flag_restart_tun_on) { ... }

    connect(qApp, &QGuiApplication::commitDataRequest, this, &MainWindow::on_commitDataRequest);

           // 状态刷新定时器（UI 刷新，保留）
    auto t = new QTimer;
    connect(t, &QTimer::timeout, this, [=]() {  });
    t->start(2000);

           // 日志计数（业务，注释）
           //Deletable t = new QTimer; connect(t, &QTimer::timeout, this, [&] { NekoGui_sys::logCounter.fetchAndStoreRelaxed(0); }); t->start(1000);

           // 自动更新订阅（业务，注释）
           //Deletable TM_auto_update_subsctiption = new QTimer; ...

    // if (!NekoGui::dataStore->flag_tray)
    //     show();
    }

void MainWindow::closeEvent(QCloseEvent *event)
    {
    if (tray->isVisible())
        {
        hide();
        event->ignore();
        }
    }

MainWindow::~MainWindow()
    {
    delete ui;
    }

// ---------- Group tab manage（保留 UI 切换逻辑，业务数据注释） ----------
inline int tabIndex2GroupId(int index)
    {
    //Deletable if (NekoGui::profileManager->groupsTabOrder.length() <= index) return -1;
    //Deletable return NekoGui::profileManager->groupsTabOrder[index];
    return -1; // 占位
    }

inline int groupId2TabIndex(int gid)
    {
    //Deletable for (int key = 0; key < NekoGui::profileManager->groupsTabOrder.count(); key++)
    //Deletable     if (NekoGui::profileManager->groupsTabOrder[key] == gid) return key;
    return 0;
    }

void MainWindow::on_tabWidget_currentChanged(int index)
    {
    // if (NekoGui::dataStore->refreshing_group_list)
    //     return;
    //Deletable if (tabIndex2GroupId(index) == NekoGui::dataStore->current_group) return;
    //Deletable show_group(tabIndex2GroupId(index));
    }

void MainWindow::show_group(int gid)
    {
    //Deletable if (NekoGui::dataStore->refreshing_group) return;
    //Deletable NekoGui::dataStore->refreshing_group = true;
    //Deletable auto group = NekoGui::profileManager->GetGroup(gid);
    //Deletable if (group == nullptr) { ... return; }
    //Deletable if (NekoGui::dataStore->current_group != gid) { ... }
    //Deletable ui->tabWidget->widget(groupId2TabIndex(gid))->layout()->addWidget(ui->proxyListTable);
    //Deletable // 列宽设置（UI）
    //Deletable if (group->manually_column_width) ... else ...
    //Deletable refresh_proxy_list_impl(-1, {GroupSortMethod::ById, true});
    //Deletable NekoGui::dataStore->refreshing_group = false;
    // 占位：仅刷新表格
    //refresh_proxy_list_impl(-1, {});
    }

// ---------- dialog_message_impl（业务回调，注释） ----------
void MainWindow::dialog_message_impl(const QString &sender, const QString &info)
    {
      //Deletable // 全部业务逻辑注释
    }

// ---------- 菜单槽函数（全部注释，只留空壳） ----------
void MainWindow::on_menu_basic_settings_triggered()
    {
    //Deletable USE_DIALOG(DialogBasicSettings)
    }

void MainWindow::on_menu_manage_groups_triggered()
    {
    //Deletable USE_DIALOG(DialogManageGroups)
    }

void MainWindow::on_menu_routing_settings_triggered()
    {
    //Deletable USE_DIALOG(DialogManageRoutes)
    }

void MainWindow::on_menu_vpn_settings_triggered()
    {
    //Deletable USE_DIALOG(DialogVPNSettings)
    }

void MainWindow::on_commitDataRequest()
    {
    // 仅保留 UI 状态保存（窗口大小、分割器等）
    if (!isMaximized())
        {
        // auto olds = NekoGui::dataStore->mw_size;
        // auto news = QStringLiteral("%1x%2").arg(size().width()).arg(size().height());
        // if (olds != news)
        //     NekoGui::dataStore->mw_size = news;
        }
    //NekoGui::dataStore->splitter_state = ui->splitter->saveState().toBase64();
    // 业务数据保存注释
    //Deletable auto last_id = NekoGui::dataStore->started_id;
    //Deletable if (NekoGui::dataStore->remember_enable && last_id >= 0) NekoGui::dataStore->remember_id = last_id;
    //NekoGui::dataStore->Save();
    //Deletable NekoGui::profileManager->SaveManager();
    }

void MainWindow::on_menu_exit_triggered()
    {
    // 退出逻辑（保留基本退出，业务清理注释）
    //Deletable if (mu_exit.tryLock()) { ... }
    tray->hide();
    QCoreApplication::quit();
    }

// 以下业务函数全部注释掉
//Deletable void MainWindow::neko_set_spmode_system_proxy(bool enable, bool save) { ... }
//Deletable void MainWindow::neko_set_spmode_vpn(bool enable, bool save) { ... }
//Deletable void MainWindow::refresh_status(const QString &traffic_update) { ... } // 保留 UI 更新，但数据来源注释
//Deletable void MainWindow::refresh_groups() { ... } // 保留标签页重建，但数据注释
//Deletable void MainWindow::refresh_proxy_list(const int &id) { refresh_proxy_list_impl(id, {}); }
//Deletable void MainWindow::refresh_proxy_list_impl(const int &id, GroupSortAction groupSortAction) { ... } // 保留表格框架
//Deletable void MainWindow::refresh_proxy_list_impl_refresh_data(const int &id) { ... } // 保留表格项设置，但数据占位
//Deletable QList<std::shared_ptr<NekoGui::ProxyEntity>> MainWindow::get_now_selected_list() { return {}; }
//Deletable QList<std::shared_ptr<NekoGui::ProxyEntity>> MainWindow::get_selected_or_group() { return {}; }
//Deletable void MainWindow::keyPressEvent(QKeyEvent *event) { ... } // 保留键盘事件（Esc, Enter）
//Deletable bool MainWindow::eventFilter(QObject *obj, QEvent *event) { return QMainWindow::eventFilter(obj, event); }
//Deletable void MainWindow::start_select_mode(QObject *context, const std::function<void(int)> &callback) { ... }
//Deletable void MainWindow::refresh_connection_list(const QJsonArray &arr) { ... } // 保留表格填充
//Deletable void MainWindow::RegisterHotkey(bool unregister) { ... }
//Deletable bool MainWindow::StopVPNProcess(bool unconditional) { return false; }

// 其他槽函数（全部空实现）
//#define EMPTY_SLOT void MainWindow::on_##name() { /* 业务已注释 */ }
// EMPTY_SLOT(masterLogBrowser_customContextMenuRequested(const QPoint &pos))
// EMPTY_SLOT(menu_add_from_input_triggered)
// EMPTY_SLOT(menu_add_from_clipboard_triggered)
// EMPTY_SLOT(menu_clone_triggered)
// EMPTY_SLOT(menu_move_triggered)
// EMPTY_SLOT(menu_delete_triggered)
// EMPTY_SLOT(menu_reset_traffic_triggered)
// EMPTY_SLOT(menu_profile_debug_info_triggered)
// EMPTY_SLOT(menu_copy_links_triggered)
// EMPTY_SLOT(menu_copy_links_nkr_triggered)
// EMPTY_SLOT(menu_export_config_triggered)
// EMPTY_SLOT(display_qr_link(bool))
// EMPTY_SLOT(menu_scan_qr_triggered)
// EMPTY_SLOT(menu_clear_test_result_triggered)
// EMPTY_SLOT(menu_select_all_triggered)
// EMPTY_SLOT(menu_delete_repeat_triggered)
// EMPTY_SLOT(menu_remove_unavailable_triggered)
// EMPTY_SLOT(menu_update_subscription_triggered)
// EMPTY_SLOT(menu_resolve_domain_triggered)
// EMPTY_SLOT(proxyListTable_itemDoubleClicked(QTableWidgetItem *item))
// EMPTY_SLOT(proxyListTable_customContextMenuRequested(const QPoint &pos))

// 注意：show_log_impl 保留，但日志来源已注释，可留空
void MainWindow::show_log_impl(const QString &log)
    {
      // 如果希望 UI 显示占位日志，可保留追加，但业务日志已无
      // 这里留空
    }

// 其他未列出的函数在头文件中已注释声明，此处不再实现
// 如有遗漏，请自行补充空实现