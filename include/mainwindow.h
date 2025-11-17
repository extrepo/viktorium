#pragma once

#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QSharedPointer>
#include <QThread>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QStyledItemDelegate>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

private:
    QPushButton* stopButton;


signals:

public slots:

private slots:
};

