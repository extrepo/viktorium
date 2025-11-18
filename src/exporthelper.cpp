#include "exporthelper.h"
#include <QApplication>
#include "databasemanager.h"

bool ExportHelper::exportQuiz(quint64 id, QWidget *parent)
{
    QString dirPath = QFileDialog::getExistingDirectory(parent,
                                                        "Выберите каталог для экспорта квиза",
                                                        QDir::homePath(),
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dirPath.isEmpty()) {
        QMessageBox::information(parent, "Экспорт отменён", "Каталог не выбран.");
        return false;
    }

    QDir outDir(dirPath);
    if (!outDir.entryList(QDir::Files).isEmpty()) {
        // В каталоге есть файлы
        QMessageBox::information(parent,
                         "Экспорт отменён",
                         "Выбранный каталог не пуст");
        return false;
    }

    QDir templateDir = QDir(QDir(QApplication::applicationDirPath()).filePath("vikatemplates"));

    // 1 страница
    QString welcome = readFile(templateDir.filePath("welcome.html"));
    writeFile(outDir.filePath("welcome.html"), welcome);

    // Остальные страницы
    QString templ = readFile(templateDir.filePath("template1.html"));
    DatabaseManager* db = &DatabaseManager::instance();
    auto quiz = db->getQuiz(id);
    QString quizName = quiz["name"].toString();
    int quizTimer = quiz["timer"].toInt();
    int questionNumber = 1;
    QVector<QVariantMap> questions = db->listQuestionsByQuiz(id);
    for(auto& q : questions) {
        QString s = QString("<script>"
            "const sample = {"
            "id: \"Q-%1\","
            "title: \"%2\","
            "topic: \"\","
            "points: %3,"
            "time_seconds: %4,"
            "text: \"%5\","
            "correct_id: \"%6\","
            "next_href: \"%7.html\""
            "options: [").arg(questionNumber).arg(quizName).arg(q["points"].toString())
            .arg(quizTimer).arg(q["text"].toString()).arg(q["answer"].toString().arg(questionNumber+1));
            int answerNumber = 1;
            for(auto& a : db->listAnswersByQuestion(q["question_id"].toInt())) {
                if(answerNumber != 1) s += ",";
                s += QString("{id: \"%1\", text: \"%2\"}").arg(answerNumber).arg(a["text"].toString());
                answerNumber++;
            }
        s += QString("],"
              "};"
            "</script>\r\n");
        writeFile(outDir.filePath(QString::number(questionNumber) + ".html"), s + templ);
        questionNumber++;
    }

    // Последняя страница
    QString finish = readFile(templateDir.filePath("finish.html"));
    writeFile(outDir.filePath(QString::number(questionNumber) + ".html"), finish);

    QMessageBox::information(parent, "Готово", "Квиз сформирован");
    return true;
}

QString ExportHelper::readFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    return content;
}

bool ExportHelper::writeFile(const QString &filePath, const QString &content) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream out(&file);
    out << content;
    file.close();
    return true;
}