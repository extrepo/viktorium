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
    static bool reportQuiz(quint64 id);
    /**
     * Отчет по командам
     */
    static bool reportTeams(QDateTime dateFrom, QDateTime dateTo);
    /**
     * Отчет по участникам
     */
    static bool reportUsers(QDateTime dateFrom, QDateTime dateTo);
};
