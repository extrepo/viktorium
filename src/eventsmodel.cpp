#include "eventsmodel.h"
#include "databasemanager.h"
#include <QBrush>
#include <QWidget>

EventsModel::EventsModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int EventsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_events.size();
}

int EventsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 5; // id, title, date, type, participant
}

QVariant EventsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_events.size())
        return QVariant();

    const Event &ev = m_events[index.row()];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return ev.id;
        case 1: return ev.title;
        case 2: return ev.date.toString("dd.MM.yyyy");
        case 3:
            switch (ev.type) {
            case 0: return "индивидуальный";
            case 1: return "командный";
            }
        case 4: return ev.participantNum;
        }

    }

    return QVariant();
}

QVariant EventsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0: return "#";
        case 1: return "Название";
        case 2: return "Дата";
        case 3: return "Тип";
        case 4: return QStringLiteral("Количество\nучастников");;
        }
    }
    return QVariant();
}

Qt::ItemFlags EventsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void EventsModel::loadSampleData()
{
    beginResetModel();
    m_events.clear();

    DatabaseManager& bd = DatabaseManager::instance();
    QSqlDatabase db = bd.database(); // метод, возвращающий QSqlDatabase

    QSqlQuery query(db);
    if (!query.exec("SELECT event_id, title, time, quiz_id FROM event")) {
        qWarning() << "Failed to load events:" << query.lastError().text();
        endResetModel();
        return;
    }

    while (query.next()) {

        Event ev;
        ev.id = query.value("event_id").toInt();
        ev.title = query.value("title").toString();
        ev.date = QDate::fromString(query.value("time").toString(), "yyyy-MM-dd"); // предполагаем, что в БД хранится в формате "YYYY-MM-DD"
        ev.type = query.value("quiz_id").toInt(); // если тип зависит от quiz_id
        ev.participantNum = 0; // если нет колонки участников, можно оставить 0 или загрузить отдельно

        m_events.append(ev);
    }

    endResetModel();
}
