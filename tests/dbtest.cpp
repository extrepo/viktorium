#include "databasemanager.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    qDebug() << "Тест базы данных";
    if(!DatabaseManager::instance().open()) {
        qWarning() << "Ошибка открытия/создания базы данных";
        return 1;
    }
    if(!DatabaseManager::instance().createTables()) {
        qWarning() << "Ошибка создания таблиц";
        return -1;
    }
    qint64 id;
    if(!DatabaseManager::instance().addUser("test", "test", "test", id)) {
        qWarning() << "Ошибка добавления участника";
        return 1;
    }
    if(!DatabaseManager::instance().removeUser(id)) {
        qWarning() << "Ошибка удаления участника";
        return 1;
    }
    DatabaseManager::instance().close();
    qDebug() << "OK";
    return 0;
}
