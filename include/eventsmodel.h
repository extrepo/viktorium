#ifndef EVENTSMODEL_H
#define EVENTSMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QDate>

struct Event {
    int id;
    QString title;
    QDate date;
    int type;     // 0-training, 1-quiz, 2-testing
    int participantNum;
};

class EventsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit EventsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void loadSampleData();

private:
    QVector<Event> m_events;
};

#endif // EVENTSMODEL_H
