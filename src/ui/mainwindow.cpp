#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "Icon.hpp"

#include "adapters/qt/QtLogBridge.h"
#include "infra/logging/Log.h"

#include "dialog_basic_settings.h"

#include <QLabel>
#include <QMessageBox>
#include <QScreen>
#include <QScrollBar>
#include <QDesktopServices>
#include <QDir>
#include <QTextBlock>


void LocationDebug(const QString &which_widget)
    {
    APP_LOG_ERROR("xxxxxxxxxxxx");
    }

namespace app::ui {

MainWindow::MainWindow(app::core::AppContext &ctx, QWidget *parent)
    : QMainWindow(parent), ui_(new Ui::MainWindow), ctx_(ctx)
    {
    //themeManager->ApplyTheme(NekoGui::dataStore->theme);
    ui_->setupUi(this);

    connect(ctx_.log_bridge, &app::adapters::qt::QtLogBridge::recordReceived,
            this, &MainWindow::show_log_impl);

    connect(ui_->menu_start, &QAction::triggered, this, []()
    { LocationDebug("menu_start"); });
    connect(ui_->menu_stop, &QAction::triggered, this, []() { });

    connect(ui_->tabWidget->tabBar(), &QTabBar::tabMoved, this,
        [](int from, int to)
    {
    });
    //这是状态栏/顶部用于显示"运行状态"（比如内核是否在运行）的 QLabel。
    //给它装事件过滤器，通常是因为普通 QLabel 默认不支持点击交互（没有 clicked 信号）
    //所以作者用 eventFilter 拦截鼠标点击事件（QEvent::MouseButtonPress 之类）
    //实现"点击这个标签可以触发某个动作"（例如点击后启动/停止内核，或弹出运行状态详情）
    ui_->label_running->installEventFilter(this);
    //同理，这是显示"入站/监听地址端口"信息的 QLabel
    ui_->label_inbound->installEventFilter(this);
    //这是日志区域和主界面之间的分隔条 QSplitter
    ui_->splitter->installEventFilter(this);

    // 顶部工具栏（UI）
    ui_->toolButton_program->setMenu(ui_->menu_program);
    ui_->toolButton_preferences->setMenu(ui_->menu_preferences);
    ui_->toolButton_server->setMenu(ui_->menu_server);
    ui_->menubar->setVisible(false);

    connect(ui_->toolButton_update, &QToolButton::clicked, this, [] {  });
    connect(ui_->toolButton_url_test, &QToolButton::clicked, this, [] { });

    //表格UI
    ui_->proxyListTable->callback_save_order = [] { };
    //ui->proxyListTable->refresh_data = [=](int id) { refresh_proxy_list_impl_refresh_data(id); };
    connect(ui_->proxyListTable->horizontalHeader(), &QHeaderView::sectionClicked, this, [](int logicalIndex)
        {
        });
    connect(ui_->proxyListTable->horizontalHeader(), &QHeaderView::sectionResized, this,
        [](int logicalIndex, int oldSize, int newSize)
        {
        });

    ui_->proxyListTable->verticalHeader()->setDefaultSectionSize(24);

    // 刷新标签页（UI）
    //this->refresh_groups();

    if (Icon::NONE != icon_status_)
        QApplication::setWindowIcon(Icon::GetTrayIcon(Icon::NONE));
    // 托盘（UI）
    tray_ = new QSystemTrayIcon(this);
    tray_->setIcon(Icon::GetTrayIcon(Icon::NONE));
    tray_->setContextMenu(ui_->menu_program);
    tray_->show();
    connect(tray_, &QSystemTrayIcon::activated, this,
        [this](QSystemTrayIcon::ActivationReason reason)
        {
        if (reason == QSystemTrayIcon::Trigger)
            {
            if(this->isVisible())
                hide();
            else
                ActivateWindow(this);
            }
        });

    connect(ui_->menu_open_config_folder, &QAction::triggered, this, []
        {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::currentPath()));
        });
    ui_->menu_program_preference->addActions(ui_->menu_preferences->actions());
    connect(ui_->menu_add_from_clipboard2, &QAction::triggered, ui_->menu_add_from_clipboard, &QAction::trigger);
    connect(ui_->actionRestart_Proxy, &QAction::triggered, this, [] {  });
    connect(ui_->actionRestart_Program, &QAction::triggered, this, [] {  });
    connect(ui_->actionShow_window, &QAction::triggered, this,
            [trayPtr = this->tray_] { trayPtr->activated(QSystemTrayIcon::ActivationReason::Trigger); });

    //既是program菜单，同时也是托盘图标的右键菜单，在显示前先要把程序的状态更新到ui上去
    connect(ui_->menu_program, &QMenu::aboutToShow, this, []()
        {
        // 菜单项状态更新（纯 UI 显示）
        //ui->actionRemember_last_proxy->setChecked(NekoGui::dataStore->remember_enable);
        //ui->actionStart_with_system->setChecked(AutoRun_IsEnabled()); // 系统自启动状态（轻量，可保留）
        //ui->actionAllow_LAN->setChecked(QStringList{"::", "0.0.0.0"}.contains(NekoGui::dataStore->inbound_address));
        });

    connect(ui_->actionStart_with_system, &QAction::triggered, this, [](bool checked)
        {
        //AutoRun_SetEnabled(checked);
        });

    // VPN / 系统代理复选框（UI 状态）
    connect(ui_->checkBox_VPN, &QCheckBox::clicked, this, [=](bool checked)
        {
        //Deletable neko_set_spmode_vpn(checked);
        });
    connect(ui_->checkBox_SystemProxy, &QCheckBox::clicked, this, [=](bool checked)
        {
        //Deletable neko_set_spmode_system_proxy(checked);
        });

    connect(ui_->menu_spmode, &QMenu::aboutToShow, this, [=]()
        {
        // ui->menu_spmode_disabled->setChecked(!(NekoGui::dataStore->spmode_system_proxy || NekoGui::dataStore->spmode_vpn));
        // ui->menu_spmode_system_proxy->setChecked(NekoGui::dataStore->spmode_system_proxy);
        // ui->menu_spmode_vpn->setChecked(NekoGui::dataStore->spmode_vpn);
        });

    //connect(ui->menu_spmode_system_proxy, &QAction::triggered, ...);
    //connect(ui->menu_spmode_vpn, &QAction::triggered, ...);
    //connect(ui->menu_spmode_disabled, &QAction::triggered, ...);
    //connect(ui->menu_qr, &QAction::triggered, ...);
    //connect(ui->menu_tcp_ping, &QAction::triggered, ...);
    //connect(ui->menu_url_test, &QAction::triggered, ...);
    //connect(ui->menu_full_test, &QAction::triggered, ...);
    //connect(ui->menu_stop_testing, &QAction::triggered, ...);


    connect(ui_->menu_server, &QMenu::aboutToHide, this, []
        {
        //setTimeout([=] { set_selected_or_group(2); }, this, 200);
        });
    //set_selected_or_group(2);

    connect(ui_->menu_share_item, &QMenu::aboutToShow, this, [ui=this->ui_]
        {
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

    connect(qApp, &QGuiApplication::commitDataRequest, this, &MainWindow::on_commitDataRequest);

    // if (!NekoGui::dataStore->flag_tray)
    //     show();
    }

void MainWindow::closeEvent(QCloseEvent *event)
    {
    if (tray_->isVisible())
        {
        hide();
        event->ignore();
        }
    }

MainWindow::~MainWindow()
    {
    delete ui_;
    }

//用户点击顶部不同的分组标签页切换分组时，Qt 会触发 currentChanged 信号进而调用 on_tabWidget_currentChanged，此时就需要用 tabIndex2GroupId 把"点了第几个 tab"翻译成"这是哪个分组的 ID"，然后调用 show_group 加载并展示该分组下的代理列表。此外，新建/删除/重新排序分组、程序启动恢复上次打开的分组等场景中，refresh_groups() 也会用到这两个转换函数来让 tab 显示顺序和分组数据保持一致
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

void MainWindow::on_proxyListTable_customContextMenuRequested(const QPoint &pos)
    {
    ui_->menu_server->popup(
        ui_->proxyListTable->viewport()->mapToGlobal(pos)); // 弹出菜单
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

void MainWindow::on_menu_basic_settings_triggered()
    {
    DialogBasicSettings dlg(*ctx_.log, *ctx_.store, this);
    if (dlg.exec() == QDialog::Accepted)
        {
        ctx_.reload_logging();
        APP_LOG_INFO("logging reconfigured from dialog");
        }
    }

void MainWindow::on_menu_manage_groups_triggered()
    {
    //Deletable USE_DIALOG(DialogManageGroups)
    }

void MainWindow::on_menu_vpn_settings_triggered()
    {
    //Deletable USE_DIALOG(DialogVPNSettings)
    }

//在程序退出/会话结束前把当前窗口状态和数据持久化保存到磁盘
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
    tray_->hide();
    QCoreApplication::quit();
    }

void MainWindow::show_log_impl(const QString &log)
    {
    auto *log_document = ui_->masterLogBrowser->document();
    auto lines = SplitLines(log.trimmed());
    if (lines.isEmpty())
        return;

    QStringList newLines;
    QStringList log_ignore = {};
    for (const auto &line : lines)
        {
        bool showThisLine = true;
        for (const auto &str : log_ignore)
            {
            if (line.contains(str))
                {
                showThisLine = false;
                break;
                }
            }
        if (showThisLine)
            newLines << line;
        }
    if (newLines.isEmpty())
        return;

    FastAppendTextDocument(newLines.join("\n"), log_document);
    // From https://gist.github.com/jemyzhang/7130092
    auto block = log_document->begin();

    while (block.isValid())
        {
        if (log_document->blockCount() > 200)
            {
            QTextCursor cursor(block);
            block = block.next();
            cursor.select(QTextCursor::BlockUnderCursor);
            cursor.movePosition(QTextCursor::NextCharacter,
                QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
            continue;
            }
        break;
        }
    }

} //namespace app::ui