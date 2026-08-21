#include <csignal>

#include <QApplication>
#include <QDir>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMessageBox>
#include <QStandardPaths>
#include <QThread>
#include <QTranslator>

#include "ui/mainwindow.h"

void signal_handler(int signum)
    {
    // if (qApp)
    //     {
    //     GetMainWindow()->on_commitDataRequest();
    //     qApp->exit();
    //     }
    }

QTranslator *trans = nullptr;
QTranslator *trans_qt = nullptr;

void loadTranslate(const QString &locale)
    {
    if (trans != nullptr)
        {
        trans->deleteLater();
        }
    if (trans_qt != nullptr)
        {
        trans_qt->deleteLater();
        }
    //
    trans = new QTranslator;
    trans_qt = new QTranslator;
    QLocale::setDefault(QLocale(locale));
    //
    if (trans->load(":/translations/" + locale + ".qm"))
        {
        QCoreApplication::installTranslator(trans);
        }
    if (trans_qt->load(
            QApplication::applicationDirPath() + "/qtbase_" + locale + ".qm"))
        {
        QCoreApplication::installTranslator(trans_qt);
        }
    }

int main(int argc, char *argv[])
    {
    QApplication app(argc, argv);

    // icon for no theme
    if (QIcon::themeName().isEmpty())
        {
        QIcon::setThemeName("breeze");
        }


    // Signals
    // signal(SIGTERM, signal_handler);
    // signal(SIGINT, signal_handler);


    UI_InitMainWindow();
    return QApplication::exec();
    }
