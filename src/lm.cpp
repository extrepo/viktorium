#include "lm.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QRegularExpression>

LM::LM(QObject *parent) : QObject(parent)
{
}

void LM::requestQuestion(const QString &topic)
{
    QUrl url("http://localhost:1234/v1/chat/completions");  // LM Studio API endpoint
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject message;
    message["role"] = "user";
    message["content"] =
        QString("Сгенерируй один учебный вопрос по теме '%1' и 3-4 ответа. Оцени сложность от 1 до 5."
                "Формат ответа строго JSON: "
                "{ \"question\": \"...\", \"answers\": [\"...\"], \"correct_index\": N, \"difficulty\": N }")
        .arg(topic);

    QJsonObject payload;
    payload["model"] = "gemma-3-4b-it";  // Модель LM Studio
    payload["messages"] = QJsonArray{ message };
    payload["temperature"] = 0.7;

    QNetworkReply *reply = m_manager.post(request, QJsonDocument(payload).toJson());

    connect(reply, &QNetworkReply::finished, this, &LM::onReplyFinished);
}

void LM::onReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(responseData);

    if (!doc.isObject()) {
        emit errorOccurred("Неверный JSON от модели");
        return;
    }

    // LM Studio возвращает:
    // { choices: [ { message: { content: "{...json...}" } } ] }
    QJsonObject root = doc.object();
    QJsonArray choices = root["choices"].toArray();

    if (choices.isEmpty()) {
        emit errorOccurred("Нет choices в ответе");
        return;
    }

    QString content = choices[0].toObject()["message"].toObject()["content"].toString();
    // Удаляем json впереди
    content.remove(QRegularExpression("```json|```"));
    // Удаляем блоки <think>...</think> (в т.ч. многострочные)
    content.remove(QRegularExpression("<think>.*?</think>", QRegularExpression::DotMatchesEverythingOption));

    QJsonDocument quizJson = QJsonDocument::fromJson(content.toUtf8());
    if (!quizJson.isObject()) {
        emit errorOccurred("Внутренний JSON не читается");
        return;
    }

    QVector<QVariant> result;
    result.push_back(quizJson["question"].toString());
    result.push_back(quizJson["correct_index"].toInt(0));
    result.push_back(quizJson["difficulty"].toInt(0));
    if (!quizJson["answers"].isArray()) {
        emit errorOccurred("Внутренний JSON с ошибками");
        return;
    }
    QJsonArray arr = quizJson["answers"].toArray();
    for (int i=0;i<arr.size();i++) {
        result.push_back(arr[i].toString());
    }

    emit questionReady(result);
}
