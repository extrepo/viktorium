#pragma once

#include <QObject>
#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QResource>

class ReportHelper : public QObject
{
    Q_OBJECT
public:
    /**
     * Отчет по eventId
     */
    static bool reportQuiz(quint64 id, QWidget *parent = nullptr);

protected:
    static QString readFile(const QString &filePath);
    static bool writeFile(const QString &filePath, const QString &content);
};
