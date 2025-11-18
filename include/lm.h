#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>

class LM : public QObject
{
    Q_OBJECT
public:
    explicit LM(QObject *parent = nullptr);

    void requestQuestion(const QString &topic);

signals:
    /**
     * @brief Готов ответ от LM
     * @param result
     *   [0] - вопрос
     *   [1] - номер правильного ответа
     *   [2] - сложность от 1 до 5
     *   [3..] - варианты ответов
     */
    void questionReady(const QVector<QVariant> &result);
    /**
     * Ошибка обращения к LM
     */
    void errorOccurred(const QString &error);

private slots:
    void onReplyFinished();

private:
    QNetworkAccessManager m_manager;
};
