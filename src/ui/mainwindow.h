#pragma once

#include <QMainWindow>


#include <QTableWidgetItem>
#include <QKeyEvent>
#include <QSystemTrayIcon>
#include <QPoint>

#include "Qt/NekoUtility.hpp"

#include "core/AppContext.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
    }
QT_END_NAMESPACE

namespace app::ui {

class MainWindow : public QMainWindow
    {
    Q_OBJECT

    public:
        explicit MainWindow(app::core::AppContext &ctx, QWidget *parent = nullptr);

        ~MainWindow() override;

        void show_log_impl(const QString &log);
        void refresh_proxy_list(const int &id = -1);
        void show_group(int gid);
        void refresh_groups();
        void refresh_status(const QString &traffic_update = "");

    signals:
        void profile_selected(int id);

    public slots:
        void on_commitDataRequest();
        void on_menu_exit_triggered();

    #ifndef MW_INTERFACE

    private slots:
        void on_menu_basic_settings_triggered();
        void on_menu_vpn_settings_triggered();
        void on_menu_add_from_clipboard_triggered() {}
        void on_menu_move_triggered() {}
        void on_menu_delete_triggered() {}
        void on_menu_reset_traffic_triggered() {}
        void on_menu_profile_debug_info_triggered() {}
        void on_menu_copy_links_triggered() {}
        void on_menu_copy_links_nkr_triggered() {}
        void on_menu_export_config_triggered() {}
        void on_menu_clear_test_result_triggered() {}
        void on_menu_manage_groups_triggered();
        void on_menu_select_all_triggered() {}
        void on_menu_delete_repeat_triggered() {}
        void on_menu_remove_unavailable_triggered() {}
        void on_menu_update_subscription_triggered() {}
        void on_menu_resolve_domain_triggered() {}
        void on_proxyListTable_itemDoubleClicked(QTableWidgetItem *item) {}
        void on_proxyListTable_customContextMenuRequested(const QPoint &pos);
        void on_tabWidget_currentChanged(int index);

    private:
        Ui::MainWindow *ui_;
        QSystemTrayIcon *tray_;
        int icon_status_ = -1;

        app::core::AppContext& ctx_;

        void closeEvent(QCloseEvent *event) override;

    protected:
        // bool eventFilter(QObject *obj, QEvent *event) override;

    #endif // MW_INTERFACE
    };

} //namespace app::ui
