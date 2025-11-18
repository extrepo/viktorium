#include <QCoreApplication>
#include <QJsonDocument>
#include "lm.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    LM client;
    QObject::connect(&client, &LM::questionReady,
                     [&](const QVector<QVariant> &result) {
            qDebug() << "=== Получен ответ от LM Studio ===";
            qDebug().noquote() << result[0].toString();
            qDebug().noquote() << result[1].toInt();
            qDebug().noquote() << result[2].toInt();
            for(int i=0;i<result.size()-3;i++) {
                qDebug().noquote() << i << result[i+3].toString();
            }
            app.quit();
        });

    QObject::connect(&client, &LM::errorOccurred,
        [&](const QString &err) {
            qWarning() << "Ошибка:" << err;
            app.quit();
        });

    // Тема вопроса — простая тестовая
    QString topic = "секс";

    qDebug() << "Отправка запроса для темы:" << topic;
    client.requestQuestion(topic);

    return app.exec();
    return 0;
}
