#ifndef QUIZMODEL_H
#define QUIZMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QDate>

struct Quiz {
    int id;
    QString topic;
    int timer;
};

class QuizModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit QuizModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void loadSampleData();

    bool addQuiz(const Quiz& ev);

private:
    QVector<Quiz> m_events;
};

#endif // EVENTSMODEL_H
