#include "quizmodel.h"
#include "databasemanager.h"
#include <QBrush>
#include <QWidget>

QuizModel::QuizModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int QuizModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_events.size();
}

int QuizModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 2; // id, topic
}

QVariant QuizModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_events.size())
        return QVariant();

    const Quiz &q = m_events[index.row()];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return q.id;
        case 1: return q.topic;
        }
    }

    return QVariant();
}

QVariant QuizModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0: return "#";
        case 1: return "Тема";
        }
    }
    return QVariant();
}

Qt::ItemFlags QuizModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void QuizModel::loadSampleData()
{
    beginResetModel();
    m_events.clear();

    DatabaseManager& bd = DatabaseManager::instance();
    QSqlDatabase db = bd.database(); // метод, возвращающий QSqlDatabase

    QSqlQuery query(db);
    if (!query.exec("SELECT quiz_id, topic, timer FROM quiz")) {
        qWarning() << "Failed to load events:" << query.lastError().text();
        endResetModel();
        return;
    }

    while (query.next()) {
        Quiz ev;
        ev.id = query.value("quiz_id").toInt();
        ev.topic = query.value("topic").toString();
        ev.timer = query.value("timer").toInt();

        m_events.append(ev);
    }

    endResetModel();
}

bool QuizModel::addQuiz(const Quiz& ev)
{
    DatabaseManager& bd = DatabaseManager::instance();
    qint64 id;
    bd.addQuiz(ev.topic, ev.timer, id);
    return true;
}
