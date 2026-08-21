#pragma once

#include <QMainWindow>


#ifndef MW_INTERFACE

#include <QTime>
#include <QTableWidgetItem>
#include <QKeyEvent>
#include <QSystemTrayIcon>
#include <QProcess>
#include <QTextDocument>
#include <QShortcut>
#include <QSemaphore>
#include <QMutex>


#endif

namespace NekoGui_sys {
class CoreProcess;
    }

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
    }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

           // ---------- UI 相关（保留） ----------
    void refresh_proxy_list(const int &id = -1);          // 保留声明，实现中只保留表格操作
    void show_group(int gid);                             // 保留声明，实现中只保留标签页切换
    void refresh_groups();                                // 保留声明，实现中只保留标签页重建
    void refresh_status(const QString &traffic_update = ""); // 保留声明，实现中只更新标签文本和图标
    void show_log_impl(const QString &log);               // 保留声明，实现中只追加文本（日志来源可忽略）
    void refresh_connection_list(const QJsonArray &arr);  // 保留声明，实现中只填充表格
    void RegisterHotkey(bool unregister);                 // 保留声明，实现为空

           // ---------- 业务方法（注释掉声明，但保留定义空壳，避免外部链接错误） ----------
           //Deletable void neko_start(int _id = -1);
           //Deletable void neko_stop(bool crash = false, bool sem = false);
           //Deletable void neko_set_spmode_system_proxy(bool enable, bool save = true);
           //Deletable void neko_set_spmode_vpn(bool enable, bool save = true);
           //Deletable void start_select_mode(QObject *context, const std::function<void(int)> &callback);
           //Deletable bool StopVPNProcess(bool unconditional = false);

signals:
    void profile_selected(int id);   // 信号保留（UI 可能用到）

public slots:
    // 以下槽函数均为 UI 触发，保留声明，实现为空或仅 UI 操作
    void on_commitDataRequest();
    void on_menu_exit_triggered();

#ifndef MW_INTERFACE

private slots:
    // 所有菜单槽函数保留声明，实现为空
    void on_masterLogBrowser_customContextMenuRequested(const QPoint &pos){}
    void on_menu_basic_settings_triggered();
    void on_menu_routing_settings_triggered();
    void on_menu_vpn_settings_triggered();
    void on_menu_add_from_input_triggered(){}
    void on_menu_add_from_clipboard_triggered(){}
    void on_menu_clone_triggered(){}
    void on_menu_move_triggered(){}
    void on_menu_delete_triggered(){}
    void on_menu_reset_traffic_triggered(){}
    void on_menu_profile_debug_info_triggered(){}
    void on_menu_copy_links_triggered(){}
    void on_menu_copy_links_nkr_triggered(){}
    void on_menu_export_config_triggered(){}
    void display_qr_link(bool nkrFormat = false){}
    void on_menu_scan_qr_triggered(){}
    void on_menu_clear_test_result_triggered(){}
    void on_menu_manage_groups_triggered();
    void on_menu_select_all_triggered(){}
    void on_menu_delete_repeat_triggered(){}
    void on_menu_remove_unavailable_triggered(){}
    void on_menu_update_subscription_triggered(){}
    void on_menu_resolve_domain_triggered(){}
    void on_proxyListTable_itemDoubleClicked(QTableWidgetItem *item){}
    void on_proxyListTable_customContextMenuRequested(const QPoint &pos){}
    void on_tabWidget_currentChanged(int index);

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *tray;
    QShortcut *shortcut_ctrl_f = new QShortcut(QKeySequence("Ctrl+F"), this);
    QShortcut *shortcut_esc = new QShortcut(QKeySequence("Esc"), this);

           // ---------- 业务成员（注释掉） ----------
           //Deletable NekoGui_sys::CoreProcess *core_process;
           //Deletable qint64 vpn_pid = 0;

           // ---------- UI 相关成员（保留） ----------
    bool qvLogAutoScoll = true;
    QTextDocument *qvLogDocument = new QTextDocument(this);

           // 以下为 UI 状态（可保留，但业务逻辑注释后可能不再更新）
    QString title_error;
    int icon_status = -1;

    QString traffic_update_cache;
    QTime last_test_time;
    int proxy_last_order = -1;
    bool select_mode = false;
    QMutex mu_starting;
    QMutex mu_stopping;
    QMutex mu_exit;
    QSemaphore sem_stopped;
    int exit_reason = 0;

    // ---------- 私有辅助（保留声明，实现为空） ----------
    void dialog_message_impl(const QString &sender, const QString &info);
    //void refresh_proxy_list_impl(const int &id = -1, GroupSortAction groupSortAction = {});
    void refresh_proxy_list_impl_refresh_data(const int &id = -1);

    //void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

           // ---------- 业务辅助（注释掉） ----------
           //Deletable void HotkeyEvent(const QString &key);
           //Deletable bool StartVPNProcess();
           //Deletable static void setup_grpc();
           //Deletable void speedtest_current_group(int mode, bool test_group);
           //Deletable void speedtest_current();
           //Deletable static void stop_core_daemon();
           //Deletable void CheckUpdate();

protected:
    //bool eventFilter(QObject *obj, QEvent *event) override;

#endif // MW_INTERFACE
    };

//MainWindow *mainwindow;

// inline MainWindow *GetMainWindow() {
//     return (MainWindow *) mainwindow;
//     }

void UI_InitMainWindow();