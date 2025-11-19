#include "include/databasemanager.h"

DatabaseManager::DatabaseManager(const QString &dbPath)
    : QObject(nullptr), m_dbPath(dbPath)
{
    // nothing
}

DatabaseManager::~DatabaseManager()
{
    // nothing
}

bool DatabaseManager::open()
{
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        m_db = QSqlDatabase::database();
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE");
        m_db.setDatabaseName(m_dbPath);
    }

    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    // ensure foreign keys
    QSqlQuery q(m_db);
    if (!q.exec("PRAGMA foreign_keys = ON;")) {
        m_lastError = q.lastError().text();
        return false;
    }

    return true;
}

void DatabaseManager::close()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    QString connection = m_db.connectionName();
    m_db = QSqlDatabase(); // clear
    QSqlDatabase::removeDatabase(connection);
}

bool DatabaseManager::createTables()
{
    if (!m_db.isOpen() && !open()) return false;

    QSqlQuery q(m_db);
    bool ok = true;

    // user
    ok &= q.exec(R"sql(
        CREATE TABLE IF NOT EXISTS "user" (
            user_id INTEGER PRIMARY KEY AUTOINCREMENT,
            surname TEXT,
            name TEXT,
            father_name TEXT
        );
    )sql");

    // team
    ok &= q.exec(R"sql(
        CREATE TABLE IF NOT EXISTS team (
            team_id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT
        );
    )sql");

    // team_user (many-to-many)
    ok &= q.exec(R"sql(
        CREATE TABLE IF NOT EXISTS team_user (
            user_id INTEGER NOT NULL,
            team_id INTEGER NOT NULL,
            PRIMARY KEY (user_id, team_id),
            FOREIGN KEY (user_id) REFERENCES "user"(user_id) ON DELETE CASCADE,
            FOREIGN KEY (team_id) REFERENCES team(team_id) ON DELETE CASCADE
        );
    )sql");

    // quiz
    ok &= q.exec(R"sql(
        CREATE TABLE IF NOT EXISTS quiz (
            quiz_id INTEGER PRIMARY KEY AUTOINCREMENT,
            topic TEXT,
            timer INTEGER
        );
    )sql");

    // event
    ok &= q.exec(R"sql(
        CREATE TABLE IF NOT EXISTS event (
            event_id INTEGER PRIMARY KEY AUTOINCREMENT,
            quiz_id INTEGER,
            title TEXT,
            time TEXT,
            type INTEGER, -- boolean stored as 0/1,
            FOREIGN KEY (quiz_id) REFERENCES quiz(quiz_id) ON DELETE CASCADE
        );
    )sql");

    // question
    ok &= q.exec(R"sql(
        CREATE TABLE IF NOT EXISTS question (
            question_id INTEGER PRIMARY KEY AUTOINCREMENT,
            quiz_id INTEGER,
            text TEXT,
            points INTEGER,
            answer INTEGER,
            FOREIGN KEY (quiz_id) REFERENCES quiz(quiz_id) ON DELETE CASCADE
        );
    )sql");

    // answer
    ok &= q.exec(R"sql(
        CREATE TABLE IF NOT EXISTS answer (
            answer_id INTEGER PRIMARY KEY AUTOINCREMENT,
            question_id INTEGER,
            text TEXT,
            FOREIGN KEY (question_id) REFERENCES question(question_id) ON DELETE CASCADE
        );
    )sql");

    // participant
    ok &= q.exec(R"sql(
        CREATE TABLE IF NOT EXISTS participant (
            participant_id INTEGER PRIMARY KEY AUTOINCREMENT,
            event_id INTEGER,
            user_id INTEGER,
            team_id INTEGER,
            number INTEGER,
            FOREIGN KEY (event_id) REFERENCES event(event_id) ON DELETE CASCADE,
            FOREIGN KEY (user_id) REFERENCES "user"(user_id) ON DELETE SET NULL,
            FOREIGN KEY (team_id) REFERENCES team(team_id) ON DELETE SET NULL
        );
    )sql");

    // result
    ok &= q.exec(R"sql(
        CREATE TABLE IF NOT EXISTS result (
            result_id INTEGER PRIMARY KEY AUTOINCREMENT,
            question_id INTEGER,
            participant_id INTEGER,
            event_id INTEGER,
            result INTEGER, -- boolean 0/1
            FOREIGN KEY (question_id) REFERENCES question(question_id) ON DELETE CASCADE,
            FOREIGN KEY (participant_id) REFERENCES participant(participant_id) ON DELETE CASCADE,
            FOREIGN KEY (event_id) REFERENCES event(event_id) ON DELETE CASCADE
        );
    )sql");

    if (!ok) {
        m_lastError = q.lastError().text();
    }

    return ok;
}

// ---------- Utility helpers ----------
bool DatabaseManager::execPrepared(QSqlQuery &query, const QVariantList &bindValues)
{
    for (int i = 0; i < bindValues.size(); ++i) {
        query.bindValue(i, bindValues.at(i));
    }
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

QVariantMap DatabaseManager::recordToMap(const QSqlRecord &rec)
{
    QVariantMap m;
    for (int i = 0; i < rec.count(); ++i) {
        m.insert(rec.fieldName(i), rec.value(i));
    }
    return m;
}

// ---------- USER ----------
bool DatabaseManager::addUser(const QString &surname, const QString &name, const QString &fatherName, qint64 &outId)
{
    if (!m_db.isOpen() && !open()) return false;

    QSqlQuery q(m_db);
    q.prepare("INSERT INTO \"user\" (surname, name, father_name) VALUES (?, ?, ?);");
    if (!execPrepared(q, {surname, name, fatherName})) return false;
    outId = q.lastInsertId().toLongLong();
    return true;
}

QVariantMap DatabaseManager::getUser(qint64 userId)
{
    QVariantMap empty;
    if (!m_db.isOpen() && !open()) return empty;

    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM \"user\" WHERE user_id = ?;");
    if (!execPrepared(q, {userId})) return empty;
    if (q.next()) return recordToMap(q.record());
    return empty;
}

int DatabaseManager::getUser(const QString &surname, const QString &name, const QString &fatherName)
{
    QVariantMap empty;
    if (!m_db.isOpen() && !open()) return -1;

    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM \"user\" WHERE surname = ? and name = ? and father_name = ?;");
    if (!execPrepared(q, {surname, name, fatherName})) return -1;
    if (q.next()) return recordToMap(q.record())["user_id"].toInt();
    return -1;
}

QVector<QVariantMap> DatabaseManager::listUsers()
{
    QVector<QVariantMap> res;
    if (!m_db.isOpen() && !open()) return res;
    QSqlQuery q("SELECT * FROM \"user\";", m_db);
    while (q.next()) res.append(recordToMap(q.record()));
    return res;
}

bool DatabaseManager::updateUser(qint64 userId, const QString &surname, const QString &name, const QString &fatherName)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("UPDATE \"user\" SET surname = ?, name = ?, father_name = ? WHERE user_id = ?;");
    return execPrepared(q, {surname, name, fatherName, userId});
}

bool DatabaseManager::removeUser(qint64 userId)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM \"user\" WHERE user_id = ?;");
    return execPrepared(q, {userId});
}

// ---------- TEAM ----------
bool DatabaseManager::addTeam(const QString &title, qint64 &outId)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO team (title) VALUES (?);");
    if (!execPrepared(q, {title})) return false;
    outId = q.lastInsertId().toLongLong();
    return true;
}

QVariantMap DatabaseManager::getTeam(qint64 teamId)
{
    QVariantMap empty;
    if (!m_db.isOpen() && !open()) return empty;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM team WHERE team_id = ?;");
    if (!execPrepared(q, {teamId})) return empty;
    if (q.next()) return recordToMap(q.record());
    return empty;
}

QVector<QVariantMap> DatabaseManager::listTeams()
{
    QVector<QVariantMap> v;
    if (!m_db.isOpen() && !open()) return v;
    QSqlQuery q("SELECT * FROM team;", m_db);
    while (q.next()) v.append(recordToMap(q.record()));
    return v;
}

QVector<QVariantMap> DatabaseManager::listTeamMembers(qint64 teamId)
{
    QVector<QVariantMap> v;
    if (!m_db.isOpen() && !open()) return v;
    QSqlQuery q("SELECT * FROM participant where team_id = ?;", m_db);
    if (!execPrepared(q, {teamId})) return v;
    while (q.next()) v.append(recordToMap(q.record()));
    return v;
}

QVector<QVariantMap> DatabaseManager::listTeamUsers(qint64 teamId)
{
    QVector<QVariantMap> v;
    if (!m_db.isOpen() && !open()) return v;
    QSqlQuery q("SELECT * FROM team_user where team_id = ?;", m_db);
    if (!execPrepared(q, {teamId})) return v;
    while (q.next()) v.append(recordToMap(q.record()));
    return v;
}

bool DatabaseManager::updateTeam(qint64 teamId, const QString &title)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("UPDATE team SET title = ? WHERE team_id = ?;");
    return execPrepared(q, {title, teamId});
}

bool DatabaseManager::removeTeam(qint64 teamId)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM team WHERE team_id = ?;");
    return execPrepared(q, {teamId});
}

// ---------- team_user ----------
bool DatabaseManager::addTeamUser(qint64 userId, qint64 teamId)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("INSERT OR IGNORE INTO team_user (user_id, team_id) VALUES (?, ?);");
    return execPrepared(q, {userId, teamId});
}

QVector<QVariantMap> DatabaseManager::listTeamUsers()
{
    QVector<QVariantMap> v;
    if (!m_db.isOpen() && !open()) return v;
    QSqlQuery q("SELECT * FROM team_user;", m_db);
    while (q.next()) v.append(recordToMap(q.record()));
    return v;
}

bool DatabaseManager::removeTeamUser(qint64 userId, qint64 teamId)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM team_user WHERE user_id = ? AND team_id = ?;");
    return execPrepared(q, {userId, teamId});
}

// ---------- quiz ----------
bool DatabaseManager::addQuiz(const QString &topic, qint64 timer, qint64 &outId)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO quiz (topic, timer) VALUES (?, ?);");
    if (!execPrepared(q, {topic, timer })) return false;
    outId = q.lastInsertId().toLongLong();
    return true;
}

QVariantMap DatabaseManager::getQuiz(qint64 quizId)
{
    QVariantMap empty;
    if (!m_db.isOpen() && !open()) return empty;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM quiz WHERE quiz_id = ?;");
    if (!execPrepared(q, {quizId})) return empty;
    if (q.next()) return recordToMap(q.record());
    return empty;
}

QVector<QVariantMap> DatabaseManager::listQuizzes()
{
    QVector<QVariantMap> v;
    if (!m_db.isOpen() && !open()) return v;
    QSqlQuery q("SELECT * FROM quiz;", m_db);
    while (q.next()) v.append(recordToMap(q.record()));
    return v;
}

bool DatabaseManager::updateQuiz(qint64 quizId, const QString &topic, qint64 timer)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("UPDATE quiz SET topic = ?, timer = ? WHERE quiz_id = ?;");
    return execPrepared(q, {topic, timer, quizId });
}

bool DatabaseManager::removeQuiz(qint64 quizId)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM quiz WHERE quiz_id = ?;");
    return execPrepared(q, {quizId});
}

// ---------- question ----------
bool DatabaseManager::addQuestion(qint64 quizId, const QString &text, qint64 points, qint64 answerId, qint64 &outId)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO question (quiz_id, text, points, answer) VALUES (?, ?, ?, ?);");
    if (!execPrepared(q, {quizId, text, points, answerId == 0 ? QVariant(QVariant::Int) : QVariant(answerId)})) return false;
    outId = q.lastInsertId().toLongLong();
    return true;
}

QVariantMap DatabaseManager::getQuestion(qint64 questionId)
{
    QVariantMap empty;
    if (!m_db.isOpen() && !open()) return empty;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM question WHERE question_id = ?;");
    if (!execPrepared(q, {questionId})) return empty;
    if (q.next()) return recordToMap(q.record());
    return empty;
}

QVector<QVariantMap> DatabaseManager::listQuestionsByQuiz(qint64 quizId)
{
    QVector<QVariantMap> v;
    if (!m_db.isOpen() && !open()) return v;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM question WHERE quiz_id = ?;");
    if (!execPrepared(q, {quizId})) return v;
    while (q.next()) v.append(recordToMap(q.record()));
    return v;
}

bool DatabaseManager::updateQuestion(qint64 questionId, qint64 quizId, const QString &text, qint64 points, qint64 answerId)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("UPDATE question SET quiz_id = ?, text = ?, points = ?, answer = ? WHERE question_id = ?;");
    return execPrepared(q, {quizId, text, points, answerId == 0 ? QVariant(QVariant::Int) : QVariant(answerId), questionId});
}

bool DatabaseManager::removeQuestion(qint64 questionId)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM question WHERE question_id = ?;");
    return execPrepared(q, {questionId});
}

// ---------- answer ----------
bool DatabaseManager::addAnswer(qint64 questionId, const QString &text, qint64 &outId)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO answer (question_id, text) VALUES (?, ?);");
    if (!execPrepared(q, {questionId, text})) return false;
    outId = q.lastInsertId().toLongLong();
    return true;
}

QVariantMap DatabaseManager::getAnswer(qint64 answerId)
{
    QVariantMap empty;
    if (!m_db.isOpen() && !open()) return empty;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM answer WHERE answer_id = ?;");
    if (!execPrepared(q, {answerId})) return empty;
    if (q.next()) return recordToMap(q.record());
    return empty;
}

QVector<QVariantMap> DatabaseManager::listAnswersByQuestion(qint64 questionId)
{
    QVector<QVariantMap> v;
    if (!m_db.isOpen() && !open()) return v;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM answer WHERE question_id = ?;");
    if (!execPrepared(q, {questionId})) return v;
    while (q.next()) v.append(recordToMap(q.record()));
    return v;
}

bool DatabaseManager::updateAnswer(qint64 answerId, qint64 questionId, const QString &text)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("UPDATE answer SET question_id = ?, text = ? WHERE answer_id = ?;");
    return execPrepared(q, {questionId, text, answerId});
}

bool DatabaseManager::removeAnswer(qint64 answerId)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM answer WHERE answer_id = ?;");
    return execPrepared(q, {answerId});
}

// ---------- participant ----------
bool DatabaseManager::addParticipant(qint64 eventId, qint64 userId, qint64 teamId, int number, qint64 &outId)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO participant (event_id, user_id, team_id, number) VALUES (?, ?, ?, ?);");
    if (!execPrepared(q, {eventId, userId == 0 ? QVariant(QVariant::LongLong) : QVariant(userId), teamId == 0 ? QVariant(QVariant::LongLong) : QVariant(teamId), number})) return false;
    outId = q.lastInsertId().toLongLong();
    return true;
}

QVariantMap DatabaseManager::getParticipant(qint64 participantId)
{
    QVariantMap empty;
    if (!m_db.isOpen() && !open()) return empty;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM participant WHERE participant_id = ?;");
    if (!execPrepared(q, {participantId})) return empty;
    if (q.next()) return recordToMap(q.record());
    return empty;
}

QVector<QVariantMap> DatabaseManager::listParticipantsByEvent(qint64 eventId)
{
    QVector<QVariantMap> v;
    if (!m_db.isOpen() && !open()) return v;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM participant WHERE event_id = ?;");
    if (!execPrepared(q, {eventId})) return v;
    while (q.next()) v.append(recordToMap(q.record()));
    return v;
}

bool DatabaseManager::updateParticipant(qint64 participantId, qint64 quizId, qint64 userId, qint64 teamId, int number)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("UPDATE participant SET quiz_id = ?, user_id = ?, team_id = ?, number = ? WHERE participant_id = ?;");
    return execPrepared(q, {quizId, userId == 0 ? QVariant(QVariant::LongLong) : QVariant(userId), teamId == 0 ? QVariant(QVariant::LongLong) : QVariant(teamId), number, participantId});
}

bool DatabaseManager::removeParticipant(qint64 participantId)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM participant WHERE participant_id = ?;");
    return execPrepared(q, {participantId});
}

bool DatabaseManager::removeParticipant(qint64 userId, quint64 eventId)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM participant WHERE user_id = ? and event_id = ?;");
    return execPrepared(q, {userId, eventId});
}

// ---------- result ----------
bool DatabaseManager::addResult(qint64 questionId, qint64 participantId, qint64 eventId, bool result, qint64 &outId)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("INSERT OR REPLACE INTO result (question_id, participant_id, event_id, result) VALUES (?, ?, ?);");
    if (!execPrepared(q, {questionId, participantId, eventId, result ? 1 : 0})) return false;
    outId = q.lastInsertId().toLongLong();
    return true;
}

QVariantMap DatabaseManager::getResult(qint64 resultId)
{
    QVariantMap empty;
    if (!m_db.isOpen() && !open()) return empty;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM result WHERE result_id = ?;");
    if (!execPrepared(q, {resultId})) return empty;
    if (q.next()) return recordToMap(q.record());
    return empty;
}

QVector<QVariantMap> DatabaseManager::listResultsByParticipant(qint64 participantId)
{
    QVector<QVariantMap> v;
    if (!m_db.isOpen() && !open()) return v;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM result WHERE participant_id = ?;");
    if (!execPrepared(q, {participantId})) return v;
    while (q.next()) v.append(recordToMap(q.record()));
    return v;
}

QVector<QVariantMap> DatabaseManager::listResultsByQuestion(qint64 questionId)
{
    QVector<QVariantMap> v;
    if (!m_db.isOpen() && !open()) return v;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM result WHERE question_id = ?;");
    if (!execPrepared(q, {questionId})) return v;
    while (q.next()) v.append(recordToMap(q.record()));
    return v;
}

bool DatabaseManager::updateResult(qint64 resultId, bool result)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE result SET result = ? WHERE result_id = ?;");
    return execPrepared(q, {result, resultId});
}

bool DatabaseManager::removeResult(qint64 resultId)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM result WHERE result_id = ?;");
    return execPrepared(q, {resultId});
}

bool DatabaseManager::addEvent(qint64 quizId, const QString& title, const QDateTime &time, int type, qint64 &outId)
{
    if (!m_db.isOpen() && !open()) return false;

    QSqlQuery q(m_db);
    q.prepare("INSERT INTO event (quiz_id, title, time, type) VALUES (?, ?, ?, ?);");
    if (!execPrepared(q, {quizId, title, time.toSecsSinceEpoch(), type})) return false;
    outId = q.lastInsertId().toLongLong();
    return true;
}

QVariantMap DatabaseManager::getEvent(qint64 eventId)
{
    QVariantMap empty;
    if (!m_db.isOpen() && !open()) return empty;

    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM event WHERE event_id = ?;");
    if (!execPrepared(q, {eventId})) return empty;
    if (q.next()) return recordToMap(q.record());
    return empty;
}

QVariantMap DatabaseManager::getEvent(const QDateTime &time)
{
    QVariantMap empty;
    if (!m_db.isOpen() && !open()) return empty;

    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM event WHERE time = ?;");
    if (!execPrepared(q, {time})) return empty;
    if (q.next()) return recordToMap(q.record());
    return empty;
}

bool DatabaseManager::updateEvent(qint64 eventId, qint64 quizId, const QString& title, const QDateTime &time, int type)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("UPDATE event SET quiz_id = ?, title = ?, time = ?, type = ? WHERE event_id = ?;");
    return execPrepared(q, {quizId, title, time, type, eventId});
}

bool DatabaseManager::removeEvent(qint64 eventId)
{
    if (!m_db.isOpen() && !open()) return false;
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM event WHERE event_id = ?;");
    return execPrepared(q, {eventId});
}
