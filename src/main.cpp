// main.cpp — 入口：创建 QApplication，交给 Bootstrapper 完成两阶段启动。
#include "bootstrap/Bootstrapper.h"

#include <QApplication>
#include <QResource>
#include <QTimer>

int main(int argc, char **argv)
    {
    Q_INIT_RESOURCE(neko);
    Q_INIT_RESOURCE(qss);

    QApplication app(argc, argv);

    app::bootstrap::Bootstrapper bootstrapper;
    return bootstrapper.run();
    }
