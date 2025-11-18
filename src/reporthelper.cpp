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

bool ReportHelper::reportQuiz(quint64 id, QWidget *parent)
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

QString ReportHelper::readFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    return content;
}

bool ReportHelper::writeFile(const QString &filePath, const QString &content) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream out(&file);
    out << content;
    file.close();
    return true;
}