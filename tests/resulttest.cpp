#include "databasemanager.h"
#include "reporthelper.h"
#include <QDateTime>
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    qDebug() << "Тест отчета по сквизу";
    QApplication app(argc, argv);

    DatabaseManager* db = &DatabaseManager::instance();
    if(!db->open()) {
        qWarning() << "Ошибка открытия/создания базы данных";
        return 1;
    }
    if(!db->createTables()) {
        qWarning() << "Ошибка создания таблиц";
        return -1;
    }
    qint64 userId1;
    if(!db->addUser("test1", "test1", "test1", userId1)) {
        qWarning() << "Ошибка добавления физлица 1";
        return 1;
    }
    qint64 userId2;
    if(!db->addUser("test2", "test2", "test2", userId2)) {
        qWarning() << "Ошибка добавления физлица 2";
        return 1;
    }
    qint64 quizId;
    if(!db->addQuiz("Test topic", 5, quizId)) {
        qWarning() << "Ошибка добавления квиза";
        return 1;
    }
    qint64 eventId;
    if(!db->addEvent(quizId, "Event 1", QDateTime::currentDateTime(), false, eventId)) {
        qWarning() << "Ошибка добавления события";
        return 1;
    }
    qint64 participantId1;
    if(!db->addParticipant(eventId, userId1, 0, 1, participantId1)) {
        qWarning() << "Ошибка добавления участника 1";
        return 1;
    }
    qint64 participantId2;
    if(!db->addParticipant(eventId, userId2, 0, 2, participantId2)) {
        qWarning() << "Ошибка добавления участника 2";
        return 1;
    }
    qint64 questionId1;
    if(!db->addQuestion(quizId, "Question 1", 5, 2, questionId1)) {
        qWarning() << "Ошибка добавления вопроса 1";
        return 1;
    }
    qint64 questionId2;
    if(!db->addQuestion(quizId, "Question 2", 3, 2, questionId2)) {
        qWarning() << "Ошибка добавления вопроса 2";
        return 1;
    }
    qint64 resultId11;
    if(!db->addResult(questionId1, participantId1, eventId, 1, resultId11)) {
        qWarning() << "Ошибка добавления результата 11";
        return 1;
    }
    qint64 resultId12;
    if(!db->addResult(questionId2, participantId1, eventId, 0, resultId12)) {
        qWarning() << "Ошибка добавления результата 12";
        return 1;
    }
    qint64 resultId21;
    if(!db->addResult(questionId1, participantId2, eventId, 1, resultId21)) {
        qWarning() << "Ошибка добавления результата 21";
        return 1;
    }
    qint64 resultId22;
    if(!db->addResult(questionId2, participantId2, eventId, 1, resultId22)) {
        qWarning() << "Ошибка добавления результата 22";
        return 1;
    }
    ReportHelper::reportQuiz(quizId);
    ReportHelper::reportUsers(QDateTime::currentDateTime().addDays(-5), QDateTime::currentDateTime().addDays(5));
    ReportHelper::reportTeams(QDateTime::currentDateTime().addDays(-5), QDateTime::currentDateTime().addDays(5));

    db->close();
    qDebug() << "OK";
    return 0;
}
