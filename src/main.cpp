#include <QApplication>
#include <QSettings>
#include "unilog/unilog.h"
#include "utils/settings.h"
#include "utils/utils.h"
#include "mainwindow.h"

#include "databasemanager.h"

int main(int argc, char *argv[])
{
    // Настройка логирования
    UniLog::getInstance().init(Settings::logConfig(), Settings::logDir());
    G_INFO() << "Application \"" PROJECT_NAME "\" v" PROJECT_VERSION " started";
    G_INFO() << "Compiled by " << Utils::getCompiler().c_str();
    G_INFO() << "User: " << Utils::getOSUser().c_str() << "@" << Utils::getHostName().c_str() << " on " << Utils::getOS().c_str();
    auto ifaces = Utils::interfaces();
    G_INFO() << "Interfaces: ";
    for(auto &iface : ifaces) {
        if(iface.ip == "127.0.0.1") continue;
        G_INFO() << "  - " << iface.name.c_str() << ": " << iface.ip.c_str() << "/" << iface.mask.c_str();
    }

    QApplication a(argc, argv);

    DatabaseManager& db = DatabaseManager::instance();
    if (!db.open()) {
        G_ERROR() << "Failed to open database.";
        return -1;
    }
    if (!db.createTables()) {
        G_ERROR() << "Failed to create tables.";
        return -1;
    }
    G_INFO() << "База готова к работе.";

    MainWindow w;
    w.show();

    int ret = a.exec();
    UniLog::getInstance().shutdown();
    DatabaseManager::instance().close();
    return ret;
}
