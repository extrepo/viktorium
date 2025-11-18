#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "utils/settings.h"

#include <QObject>
#include <QtSql>
#include <QVariantMap>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    static DatabaseManager& instance() {
        static DatabaseManager inst(QString::fromStdString(Settings::dbDir()) + "/quiz.db");
        return inst;
    }

    bool open();
    void close();

    // init
    bool createTables();

    // --- CRUD: user ---
    bool addUser(const QString &surname, const QString &name, const QString &fatherName, qint64 &outId);
    QVariantMap getUser(qint64 userId);
    QVector<QVariantMap> listUsers();
    bool updateUser(qint64 userId, const QString &surname, const QString &name, const QString &fatherName);
    bool removeUser(qint64 userId);

    // --- CRUD: team ---
    bool addTeam(const QString &title, qint64 &outId);
    QVariantMap getTeam(qint64 teamId);
    QVector<QVariantMap> listTeams();
    bool updateTeam(qint64 teamId, const QString &title);
    bool removeTeam(qint64 teamId);

    // --- CRUD: team_user (many-to-many) ---
    bool addTeamUser(qint64 userId, qint64 teamId);
    QVector<QVariantMap> listTeamUsers();
    bool removeTeamUser(qint64 userId, qint64 teamId);

    // --- CRUD: quiz ---
    bool addQuiz(bool type, const QString &topic, qint64 timer, qint64 &outId);
    QVariantMap getQuiz(qint64 quizId);
    QVector<QVariantMap> listQuizzes();
    bool updateQuiz(qint64 quizId, bool type, const QString &topic, qint64 timer);
    bool removeQuiz(qint64 quizId);

    // --- CRUD: question ---
    bool addQuestion(qint64 quizId, const QString &text, qint64 points, qint64 answerId, qint64 &outId);
    QVariantMap getQuestion(qint64 questionId);
    QVector<QVariantMap> listQuestionsByQuiz(qint64 quizId);
    bool updateQuestion(qint64 questionId, qint64 quizId, const QString &text, qint64 points, qint64 answerId);
    bool removeQuestion(qint64 questionId);

    // --- CRUD: answer ---
    bool addAnswer(qint64 questionId, const QString &text, qint64 &outId);
    QVariantMap getAnswer(qint64 answerId);
    QVector<QVariantMap> listAnswersByQuestion(qint64 questionId);
    bool updateAnswer(qint64 answerId, qint64 questionId, const QString &text);
    bool removeAnswer(qint64 answerId);

    // --- CRUD: participant ---
    bool addParticipant(qint64 eventId, qint64 userId, qint64 teamId, int number, qint64 &outId);
    QVariantMap getParticipant(qint64 participantId);
    QVector<QVariantMap> listParticipantsByEvent(qint64 eventId);
    bool updateParticipant(qint64 participantId, qint64 eventId, qint64 userId, qint64 teamId, int number);
    bool removeParticipant(qint64 participantId);

    // --- CRUD: result ---
    bool addResult(qint64 questionId, qint64 participantId, qint64 eventId, bool result, qint64 &outId);
    QVariantMap getResult(qint64 resultId);
    QVector<QVariantMap> listResultsByParticipant(qint64 participantId);
    QVector<QVariantMap> listResultsByQuestion(qint64 questionId);
    bool updateResult(qint64 resultId, bool result);
    bool removeResult(qint64 resultId);

    // --- CRUD: event ---
    bool addEvent(qint64 quizId, const QString& title, const QDateTime &time, qint64 &outId);
    QVariantMap getEvent(qint64 eventId);
    QVariantMap getEvent(const QDateTime &time);
    bool updateEvent(qint64 eventId, qint64 quizId, const QString& title, const QDateTime &time);
    bool removeEvent(qint64 eventId);

    // --- Reports ---
    QVector<QVariantMap> resultTeams(const QDateTime dateFrom, const QDateTime dateTo);
    QVector<QVariantMap> resultUsers(const QDateTime dateFrom, const QDateTime dateTo);

    // utility
    QString lastError() const { return m_lastError; }
    QSqlDatabase database() const { return m_db; }

private:
    DatabaseManager(const QString &dbPath);
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool execPrepared(QSqlQuery &query, const QVariantList &bindValues = QVariantList());
    QVariantMap recordToMap(const QSqlRecord &rec);

    QString m_dbPath;
    QSqlDatabase m_db;
    QString m_lastError;
};

#endif // DATABASEMANAGER_H
