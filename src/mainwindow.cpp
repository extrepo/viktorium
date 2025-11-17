#include "mainwindow.h"
#include <QAction>
#include <QLabel>
#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QMenuBar>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Викториум");
    resize(651, 575);
    setMinimumSize(QSize(0, 0));
    setMaximumSize(QSize(16777215, 16777215));
    setWindowIcon(QIcon(":/icons/viktorium.ico"));

    QAction* saveConfigAction = new QAction(this);
    saveConfigAction->setText("Сохранить конфигурацию");
    QAction* exitAction = new QAction(this);
    exitAction->setText("Выход");
    QAction* exportAction = new QAction(this);
    exportAction->setText("Экспорт");
    QAction* initAction = new QAction(this);
    initAction->setText("Сброс устройств");
    QAction* deviceSettingsAction = new QAction(this);
    deviceSettingsAction->setText("Выбор устройств");
    QAction* paramsAction = new QAction(this);
    paramsAction->setText("Параметры");
    QAction* settingsAction = new QAction(this);
    settingsAction->setText("Общие настройки");
    QAction* newProject = new QAction(this);
    newProject->setText("Новый проект");
    QAction* trdAction = new QAction(this);
    trdAction->setText("Преобразователи");
    QAction* clearTable = new QAction(this);
    clearTable->setText("Очистить таблицу");

    QWidget* centralWidget = new QWidget(this);
    QGridLayout* mainLayout = new QGridLayout(centralWidget);
}
