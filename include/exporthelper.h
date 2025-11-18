#pragma once

#include <QObject>
#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QResource>

class ExportHelper : public QObject
{
    Q_OBJECT
public:
    static bool exportQuiz(quint64 id, QWidget *parent = nullptr);

protected:
    static QString readFile(const QString &filePath);
    static bool writeFile(const QString &filePath, const QString &content);
};
