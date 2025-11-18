#include "reporthelper.h"
#include <QApplication>
#include <QDesktopServices>
#include "databasemanager.h"
#include "unilog/unilog.h"

class UnicodedStream : QTextStream
{
    using QTextStream::QTextStream;
public:
    using QTextStream::setCodec;
    template<typename T>
    UnicodedStream& operator<<(T const& t)
    {
        return static_cast<UnicodedStream&>(static_cast<QTextStream&>(*this) << t);
    }
    UnicodedStream& operator<<(char const* ptr)
    {
        return static_cast<UnicodedStream&>(*this << QString(ptr));
    }
};

bool ReportHelper::reportQuiz(quint64 id)
{
    QString fileName =  QString::fromStdString(Settings::dbDir()) + QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss") + ".html";
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        G_ERROR() << "Could not create file:" << file.errorString();
        QMessageBox::critical(nullptr, "Ошибка", "Невозможно создать файл отчета");
        return false;
    }
    UnicodedStream out(&file);
    out.setCodec("UTF-8");
    out << "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Результаты</title></head><body>\r\n";
    //
    DatabaseManager* db = &DatabaseManager::instance();
    auto event = db->getEvent(id);
    auto quiz = db->getQuiz(event["quiz_id"].toInt());
    out << quiz["text"].toString() << " (" << (quiz["type"].toInt() == 1 ? "Групповой)" : "Индивидуальный)");
    // Считаем результат
    QMap<int, int> resMap;
    auto questions = db->listQuestionsByQuiz(id);
    for(auto& q : questions) {
        // Смотрим на результаты
        for(auto& r : db->listResultsByQuestion(q["question_id"].toInt())) {
            int participant = r["participant_id"].toInt();
            if(r["result"].toInt()) {
                if(!resMap.contains(participant)) resMap[participant] = 0;
                resMap[participant] += q["points"].toInt();
            }
        }
    }
    // Копируем в вектор пар
    QVector<QPair<int,int>> tv;
    for(auto it = resMap.begin(); it != resMap.end(); ++it) { 
        tv.push_back(qMakePair(it.key(), it.value()));
    }
    // Сортируем по значению в порядке убывания
    std::sort(tv.begin(), tv.end(), [](const QPair<int, int>& a, const QPair<int, int>& b) {
        return a.second > b.second; // убывание значений
    });
    // Выводим результат
    out << "<table border=1><th>";
    out << "<td>" <<(quiz["type"].toInt() == 1 ? "Команда" : "Участник") << "</td>";
    out << "<td>Набрано баллов</td></th>\r\n";
    for (const auto& p : tv) {
        out << "<tr>";
        auto participant = db->getParticipant(p.second);
        out << "<td>" << participant["number"].toString() << "</td>";
        out << "<td>" << p.first << "</td>";
        out << "</tr>\r\n";
    }
    out << "</table>\r\n";
    // Завершение
    out << "</body></html>\r\n";
    file.close();
    // Открываем браузер
    QUrl url = QUrl::fromLocalFile(fileName);
    QDesktopServices::openUrl(url);
    return true;
}

bool ReportHelper::reportTeams(QDateTime dateFrom, QDateTime dateTo)
{
    QString fileName =  QString::fromStdString(Settings::dbDir()) + QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss") + ".html";
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        G_ERROR() << "Could not create file:" << file.errorString();
        QMessageBox::critical(nullptr, "Ошибка", "Невозможно создать файл отчета");
        return false;
    }
    UnicodedStream out(&file);
    out.setCodec("UTF-8");
    out << "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Результаты команды</title></head><body>\r\n";
    //
    DatabaseManager* db = &DatabaseManager::instance();
    auto rteams = db->resultTeams(dateFrom, dateTo);
    QString teamTitle;
    int quizId = -1;
    int games = 0;
    int points = 0;
    int totalPoints = 0;
    out << "Командные результаты с " << dateFrom.toString("dd.MM.yyyy") << " по " << dateTo.toString("dd.MM.yyyy");
    out << "<table border=1>";
    out << "<th><td>Команда</td><td>Игры</td><td>Баллы</td><td>%</td></th>\r\n";
    for(auto& r : rteams) {
        if(r["title"].toString() != teamTitle) {
            if(!teamTitle.isEmpty()) {
                out << "<tr>";
                out << "<td>"<< teamTitle << "</td>";
                out << "<td>" << games << "</td>";
                out << "<td>" << points << "</td>";
                out << "<td>" << (totalPoints > 0 ? 100*points/totalPoints : 0) << "</td>";
                out << "</tr>\r\n";
            }
            teamTitle = r["title"].toString();
            quizId = -1;
            games = 0;
            points = 0;
            totalPoints = 0;
        }
        if(quizId != r["quiz_id"].toInt()) {
            games++;
            quizId = r["quiz_id"].toInt();
        }
        totalPoints += r["points"].toInt();
        if(r["result"].toInt() > 0) points += r["points"].toInt();
    }
    if(!teamTitle.isEmpty()) {
        out << "<tr>";
        out << "<td>"<< teamTitle << "</td>";
        out << "<td>" << games << "</td>";
        out << "<td>" << points << "</td>";
        out << "<td>" << (totalPoints > 0 ? 100*points/totalPoints : 0) << "</td>";
        out << "</tr>\r\n";
    }
    out << "</table></body></html>\r\n";
    file.close();
    // Открываем браузер
    QUrl url = QUrl::fromLocalFile(fileName);
    QDesktopServices::openUrl(url);
    return true;
}

static bool reportUsers(QDateTime dateFrom, QDateTime dateTo)
{
    QString fileName =  QString::fromStdString(Settings::dbDir()) + QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss") + ".html";
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        G_ERROR() << "Could not create file:" << file.errorString();
        QMessageBox::critical(nullptr, "Ошибка", "Невозможно создать файл отчета");
        return false;
    }
    UnicodedStream out(&file);
    out.setCodec("UTF-8");
    out << "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Результаты участника</title></head><body>\r\n";
    //
    DatabaseManager* db = &DatabaseManager::instance();
    auto rteams = db->resultUsers(dateFrom, dateTo);
    QString userTitle;
    int userId = -1;
    int quizId = -1;
    int games = 0;
    int points = 0;
    int totalPoints = 0;
    out << "Личные результаты с " << dateFrom.toString("dd.MM.yyyy") << " по " << dateTo.toString("dd.MM.yyyy");
    out << "<table border=1>";
    out << "<th><td>Участник</td><td>Игры</td><td>Баллы</td><td>%</td></th>\r\n";
    for(auto& r : rteams) {
        if(r["user_id"].toInt() != userId) {
            if(userId != -1) {
                out << "<tr>";
                out << "<td>"<< userTitle << "</td>";
                out << "<td>" << games << "</td>";
                out << "<td>" << points << "</td>";
                out << "<td>" << (totalPoints > 0 ? 100*points/totalPoints : 0) << "</td>";
                out << "</tr>\r\n";
            }
            userId = r["user_id"].toInt();
            userTitle = r["name"].toString() + " " + r["father_name"].toString() + " " + r["surname"].toString();
            quizId = -1;
            games = 0;
            points = 0;
            totalPoints = 0;
        }
        if(quizId != r["quiz_id"].toInt()) {
            games++;
            quizId = r["quiz_id"].toInt();
        }
        totalPoints += r["points"].toInt();
        if(r["result"].toInt() > 0) points += r["points"].toInt();
    }
    if(!userTitle.isEmpty()) {
        out << "<tr>";
        out << "<td>"<< userTitle << "</td>";
        out << "<td>" << games << "</td>";
        out << "<td>" << points << "</td>";
        out << "<td>" << (totalPoints > 0 ? 100*points/totalPoints : 0) << "</td>";
        out << "</tr>\r\n";
    }
    out << "</table></body></html>\r\n";
    file.close();
    // Открываем браузер
    QUrl url = QUrl::fromLocalFile(fileName);
    QDesktopServices::openUrl(url);
    return true;
}
