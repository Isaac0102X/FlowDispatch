#include "dialog_basic_settings.h"
#include "ui_dialog_basic_settings.h"

#include "3rdparty/qv2ray/v2/ui/widgets/editors/w_JsonEditor.hpp"
// #include "fmt/Preset.hpp"
//#include "Icon.hpp"
#include "ThemeManager.hpp"
// #include "main/GuiUtils.hpp"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QStyleFactory>
#include <QTimer>

using namespace app::core;
using namespace app::infra::config;

class ExtraCoreWidget : public QWidget
    {
public:
    QString coreName;

    QLabel *label_name;
    MyLineEdit *lineEdit_path;
    QPushButton *pushButton_pick;

    explicit ExtraCoreWidget(QJsonObject *extraCore, const QString &coreName_,
        QWidget *parent = nullptr)
        : QWidget(parent)
        {
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
        connect(pushButton_pick, &QPushButton::clicked, this, [=,this]
        {
            auto fn = QFileDialog::getOpenFileName(this, QObject::tr("Select"),
                QDir::currentPath(), "", nullptr,
                QFileDialog::Option::ReadOnly);
            if (!fn.isEmpty())
                {
                lineEdit_path->setText(fn);
                }
        });
        connect(lineEdit_path, &QLineEdit::textChanged, this,
            [=,this](const QString &newTxt)
            {
            extraCore->insert(coreName, newTxt);
            });
        }
    };

DialogBasicSettings::DialogBasicSettings(
    LogConfigSection &section, IConfigStore &store, QWidget *parent)
    : QDialog(parent), section_(section), store_(store),
      ui(new Ui::DialogBasicSettings)
    {
    ui->setupUi(this);
    ui->log_level->addItems(
        QStringLiteral("trace debug info warn error fatal panic").split(" "));
    ui->mux_protocol->addItems({"h2mux", "smux", "yamux"});

    load_log_settings();

    refresh_auth();
    // connect(ui->custom_inbound_edit, &QPushButton::clicked, this, []
    //     {
    //     });

    //禁用"连接统计"设置组（该功能在此版本未实现/被锁定，灰显）
    ui->connection_statistics_box->setDisabled(true);
    //把Qt所有可用的QStyle名称（如 Fusion、windowsvista 等，随平台和插件而变）填入主题下拉框，供用户切换程序外观风格。
    ui->theme->addItems(QStyleFactory::keys());
    }

void DialogBasicSettings::load_log_settings()
    {
    const auto &settings = section_.data();
    ui->uiLogEnabled->setChecked(settings.ui_enabled);
    ui->textLogEnabled->setChecked(settings.file_enabled);
    ui->logFileSize->setValue(static_cast<int>(settings.file_max_size_mb));
    ui->logRetainedCount->setValue(
        static_cast<int>(settings.file_rotate_count));

    const auto select_log_level = [](QComboBox *combo,
                                     const std::string &configured_level)
        {
        const QString level = QString::fromStdString(configured_level);
        for (int index = 0; index < combo->count(); ++index)
            {
            if (combo->itemText(index).compare(
                    level, Qt::CaseInsensitive) == 0)
                {
                combo->setCurrentIndex(index);
                return;
                }
            }

        combo->addItem(level);
        combo->setCurrentIndex(combo->count() - 1);
        };
    select_log_level(ui->uiLogLevel, settings.ui_level);
    select_log_level(ui->textLogLevel, settings.file_level);
    }

DialogBasicSettings::~DialogBasicSettings()
    {
    delete ui;
    }

FieldMap DialogBasicSettings::collect() const
    {
    app::infra::config::FieldMap m;
    m["ui_enabled"] = ui->uiLogEnabled->isChecked();
    m["ui_level"] = ui->uiLogLevel->currentText().toLower().toStdString();
    m["file_enabled"] = ui->textLogEnabled->isChecked();
    m["file_level"] = ui->textLogLevel->currentText().toLower().toStdString();
    m["file_max_size_mb"] = static_cast<std::int64_t>(ui->logFileSize->value());
    m["file_rotate_count"] = static_cast<std::int64_t>(ui->logRetainedCount->value());
    return m;
    }

void DialogBasicSettings::accept()
    {
    section_.apply(collect(), store_);
    QDialog::accept();
    }

// slots

void DialogBasicSettings::refresh_auth() {}

void DialogBasicSettings::on_set_custom_icon_clicked() {}

void DialogBasicSettings::on_inbound_auth_clicked()
    {
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

void DialogBasicSettings::on_core_settings_clicked()
    {
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
    // auto core_box_underlying_dns_l = new QLabel(tr("Override underlying
    // DNS")); core_box_underlying_dns = new MyLineEdit;
    // core_box_underlying_dns->setText(NekoGui::dataStore->core_box_underlying_dns);
    // core_box_underlying_dns->setMinimumWidth(300);
    // layout->addWidget(core_box_underlying_dns_l, ++line, 0);
    // layout->addWidget(core_box_underlying_dns, line, 1);
    // //
    // auto core_box_enable_clash_api_l = new QLabel("Enable Clash API");
    // core_box_enable_clash_api = new QCheckBox;
    // core_box_enable_clash_api->setChecked(NekoGui::dataStore->core_box_clash_api
    // > 0); layout->addWidget(core_box_enable_clash_api_l, ++line, 0);
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
    //     NekoGui::dataStore->core_box_underlying_dns =
    //     core_box_underlying_dns->text();
    //     NekoGui::dataStore->core_box_clash_api =
    //     core_box_clash_api->text().toInt() *
    //     (core_box_enable_clash_api->isChecked() ? 1 : -1);
    //     NekoGui::dataStore->core_box_clash_api_secret =
    //     core_box_clash_api_secret->text();
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
