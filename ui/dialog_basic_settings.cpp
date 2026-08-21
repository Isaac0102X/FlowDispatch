#include "dialog_basic_settings.h"
#include "ui_dialog_basic_settings.h"

#include "3rdparty/qv2ray/v2/ui/widgets/editors/w_JsonEditor.hpp"
//#include "fmt/Preset.hpp"
#include "ui/ThemeManager.hpp"
#include "ui/Icon.hpp"
//#include "main/GuiUtils.hpp"

#include <QStyleFactory>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>

class ExtraCoreWidget : public QWidget {
public:
    QString coreName;

    QLabel *label_name;
    MyLineEdit *lineEdit_path;
    QPushButton *pushButton_pick;

    explicit ExtraCoreWidget(QJsonObject *extraCore, const QString &coreName_,
                             QWidget *parent = nullptr)
        : QWidget(parent) {
        coreName = coreName_;
        label_name = new QLabel;
        label_name->setText(coreName);
        lineEdit_path = new MyLineEdit;
        lineEdit_path->setText(extraCore->value(coreName).toString());
        pushButton_pick = new QPushButton;
        pushButton_pick->setText(QObject::tr("Select"));
        auto layout = new QHBoxLayout;
        layout->addWidget(label_name);
        layout->addWidget(lineEdit_path);
        layout->addWidget(pushButton_pick);
        setLayout(layout);
        setContentsMargins(0, 0, 0, 0);
        //
        connect(pushButton_pick, &QPushButton::clicked, this, [=] {
            auto fn = QFileDialog::getOpenFileName(this, QObject::tr("Select"), QDir::currentPath(),
                                                   "", nullptr, QFileDialog::Option::ReadOnly);
            if (!fn.isEmpty()) {
                lineEdit_path->setText(fn);
            }
        });
        connect(lineEdit_path, &QLineEdit::textChanged, this, [=](const QString &newTxt) {
            extraCore->insert(coreName, newTxt);
        });
    }
};

DialogBasicSettings::DialogBasicSettings(QWidget *parent)
    : QDialog(parent), ui(new Ui::DialogBasicSettings) {
    ui->setupUi(this);
    //ADD_ASTERISK(this);

    // Common

    ui->log_level->addItems(QStringLiteral("trace debug info warn error fatal panic").split(" "));
    ui->mux_protocol->addItems({"h2mux", "smux", "yamux"});

    refresh_auth();

    // D_LOAD_STRING(inbound_address)
    // D_LOAD_COMBO_STRING(log_level)
    // CACHE.custom_inbound = NekoGui::dataStore->custom_inbound;
    // D_LOAD_INT(inbound_socks_port)
    // D_LOAD_INT(test_concurrent)
    // D_LOAD_INT(test_download_timeout)
    // D_LOAD_STRING(test_latency_url)
    // D_LOAD_STRING(test_download_url)
    // D_LOAD_BOOL(old_share_link_format)

    connect(ui->custom_inbound_edit, &QPushButton::clicked, this, [=] {
        // C_EDIT_JSON_ALLOW_EMPTY(custom_inbound)
    });

#ifdef Q_OS_WIN
    connect(ui->sys_proxy_format, &QPushButton::clicked, this, [=] {
        bool ok;
        auto str = QInputDialog::getItem(this, ui->sys_proxy_format->text() + " (Windows)",
                                         tr("Advanced system proxy settings. Please select a format."),
                                         Preset::Windows::system_proxy_format,
                                         Preset::Windows::system_proxy_format.indexOf(NekoGui::dataStore->system_proxy_format),
                                         false, &ok);
        if (ok) NekoGui::dataStore->system_proxy_format = str;
    });
#else
    ui->sys_proxy_format->hide();
#endif

    // Style
    ui->connection_statistics_box->setDisabled(true);
    //
    // D_LOAD_BOOL(check_include_pre)
    // D_LOAD_BOOL(connection_statistics)
    // D_LOAD_BOOL(start_minimal)
    // D_LOAD_INT(max_log_line)
    //

    //
    //ui->language->setCurrentIndex(NekoGui::dataStore->language);
    connect(ui->language, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int index) {
        CACHE.needRestart = true;
    });
    //
    int built_in_len = ui->theme->count();
    ui->theme->addItems(QStyleFactory::keys());
    //
    bool ok;
    // auto themeId = NekoGui::dataStore->theme.toInt(&ok);
    // if (ok) {
    //     ui->theme->setCurrentIndex(themeId);
    // } else {
    //     ui->theme->setCurrentText(NekoGui::dataStore->theme);
    // }
    // //
    // connect(ui->theme, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int index) {
    //     if (index + 1 <= built_in_len) {
    //         themeManager->ApplyTheme(Int2String(index));
    //         NekoGui::dataStore->theme = Int2String(index);
    //     } else {
    //         themeManager->ApplyTheme(ui->theme->currentText());
    //         NekoGui::dataStore->theme = ui->theme->currentText();
    //     }
    //     repaint();
    //     mainwindow->repaint();
    //     NekoGui::dataStore->Save();
    //});

    // Subscription

    // ui->user_agent->setText(NekoGui::dataStore->user_agent);
    // ui->user_agent->setPlaceholderText(NekoGui::dataStore->GetUserAgent(true));
    // D_LOAD_BOOL(sub_use_proxy)
    // D_LOAD_BOOL(sub_clear)
    // D_LOAD_BOOL(sub_insecure)
    // D_LOAD_INT_ENABLE(sub_auto_update, sub_auto_update_enable)

    // Core

    // Mux
    // D_LOAD_INT(mux_concurrency)
    // D_LOAD_COMBO_STRING(mux_protocol)
    // D_LOAD_BOOL(mux_padding)
    // D_LOAD_BOOL(mux_default_on)

    // // Security

    // ui->utlsFingerprint->addItems(Preset::SingBox::UtlsFingerPrint);

    // D_LOAD_BOOL(skip_cert)
    // ui->utlsFingerprint->setCurrentText(NekoGui::dataStore->utlsFingerprint);
}

DialogBasicSettings::~DialogBasicSettings() {
    delete ui;
}

void DialogBasicSettings::accept() {

}

// slots

void DialogBasicSettings::refresh_auth() {

}

void DialogBasicSettings::on_set_custom_icon_clicked() {

}

void DialogBasicSettings::on_inbound_auth_clicked() {
    // auto w = new QDialog(this);
    // w->setWindowTitle(tr("Inbound Auth"));
    // auto layout = new QGridLayout;
    // w->setLayout(layout);
    // //
    // auto user_l = new QLabel(tr("Username"));
    // auto pass_l = new QLabel(tr("Password"));
    // auto user = new MyLineEdit;
    // auto pass = new MyLineEdit;
    // user->setText(NekoGui::dataStore->inbound_auth->username);
    // pass->setText(NekoGui::dataStore->inbound_auth->password);
    // //
    // layout->addWidget(user_l, 0, 0);
    // layout->addWidget(user, 0, 1);
    // layout->addWidget(pass_l, 1, 0);
    // layout->addWidget(pass, 1, 1);
    // auto box = new QDialogButtonBox;
    // box->setOrientation(Qt::Horizontal);
    // box->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    // connect(box, &QDialogButtonBox::accepted, w, [=] {
    //     NekoGui::dataStore->inbound_auth->username = user->text();
    //     NekoGui::dataStore->inbound_auth->password = pass->text();
    //     MW_dialog_message(Dialog_DialogBasicSettings, "UpdateDataStore");
    //     w->accept();
    // });
    // connect(box, &QDialogButtonBox::rejected, w, &QDialog::reject);
    // layout->addWidget(box, 2, 1);
    // //
    // w->exec();
    // w->deleteLater();
    // refresh_auth();
}

void DialogBasicSettings::on_core_settings_clicked() {
    // auto w = new QDialog(this);
    // w->setWindowTitle(software_core_name + " Core Options");
    // auto layout = new QGridLayout;
    // w->setLayout(layout);
    // //
    // auto line = -1;
    // QCheckBox *core_box_enable_clash_api;
    // MyLineEdit *core_box_clash_api;
    // MyLineEdit *core_box_clash_api_secret;
    // MyLineEdit *core_box_underlying_dns;
    // //
    // auto core_box_underlying_dns_l = new QLabel(tr("Override underlying DNS"));
    // core_box_underlying_dns = new MyLineEdit;
    // core_box_underlying_dns->setText(NekoGui::dataStore->core_box_underlying_dns);
    // core_box_underlying_dns->setMinimumWidth(300);
    // layout->addWidget(core_box_underlying_dns_l, ++line, 0);
    // layout->addWidget(core_box_underlying_dns, line, 1);
    // //
    // auto core_box_enable_clash_api_l = new QLabel("Enable Clash API");
    // core_box_enable_clash_api = new QCheckBox;
    // core_box_enable_clash_api->setChecked(NekoGui::dataStore->core_box_clash_api > 0);
    // layout->addWidget(core_box_enable_clash_api_l, ++line, 0);
    // layout->addWidget(core_box_enable_clash_api, line, 1);
    // //
    // auto core_box_clash_api_l = new QLabel("Clash API Listen Port");
    // core_box_clash_api = new MyLineEdit;
    // core_box_clash_api->setText(Int2String(std::abs(NekoGui::dataStore->core_box_clash_api)));
    // layout->addWidget(core_box_clash_api_l, ++line, 0);
    // layout->addWidget(core_box_clash_api, line, 1);
    // //
    // auto core_box_clash_api_secret_l = new QLabel("Clash API Secret");
    // core_box_clash_api_secret = new MyLineEdit;
    // core_box_clash_api_secret->setText(NekoGui::dataStore->core_box_clash_api_secret);
    // layout->addWidget(core_box_clash_api_secret_l, ++line, 0);
    // layout->addWidget(core_box_clash_api_secret, line, 1);
    // //
    // auto box = new QDialogButtonBox;
    // box->setOrientation(Qt::Horizontal);
    // box->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    // connect(box, &QDialogButtonBox::accepted, w, [=] {
    //     NekoGui::dataStore->core_box_underlying_dns = core_box_underlying_dns->text();
    //     NekoGui::dataStore->core_box_clash_api = core_box_clash_api->text().toInt() * (core_box_enable_clash_api->isChecked() ? 1 : -1);
    //     NekoGui::dataStore->core_box_clash_api_secret = core_box_clash_api_secret->text();
    //     MW_dialog_message(Dialog_DialogBasicSettings, "UpdateDataStore");
    //     w->accept();
    // });
    // connect(box, &QDialogButtonBox::rejected, w, &QDialog::reject);
    // layout->addWidget(box, ++line, 1);
    // //
    // ADD_ASTERISK(w)
    // w->exec();
    // w->deleteLater();
    // refresh_auth();
}
