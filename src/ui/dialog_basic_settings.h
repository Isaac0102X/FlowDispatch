#ifndef DIALOG_BASIC_SETTINGS_H
#define DIALOG_BASIC_SETTINGS_H

#include <QDialog>

#include "cfg_management/BasicSettings/LogConfigSection.h"
#include "cfg_management/BasicSettings/CommonSection.h"
#include "infra/config/IConfigStore.h"

namespace Ui {
    class DialogBasicSettings;
}

class DialogBasicSettings : public QDialog {
    Q_OBJECT

public:
    explicit DialogBasicSettings(app::cfg_management::LogConfigSection &section,
                                 app::cfg_management::BasicSettings::CommonSection &common,
                                 app::infra::config::IConfigStore &store,
                                 QWidget *parent = nullptr);

    ~DialogBasicSettings();

public slots:

    void accept();

private:
    app::infra::config::FieldMap collect() const;
    void load_common_settings();
    void load_log_settings();

    Ui::DialogBasicSettings *ui;
    app::cfg_management::LogConfigSection &section_;
    app::cfg_management::BasicSettings::CommonSection &common_;
    app::infra::config::IConfigStore &store_;


private slots:

    void refresh_auth();

    void on_set_custom_icon_clicked();

    void on_inbound_auth_clicked();

    void on_core_settings_clicked();
};

#endif // DIALOG_BASIC_SETTINGS_H
