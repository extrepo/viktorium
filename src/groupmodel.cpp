#include "groupmodel.h"
#include "databasemanager.h"

GroupsModel::GroupsModel(QObject* parent)
    : QAbstractListModel(parent)
{
    refreshFromDatabase();
}

int GroupsModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_groups.count();
}

QVariant GroupsModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return {};
    if (index.row() < 0 || index.row() >= m_groups.size()) return {};
    const Group &g = m_groups.at(index.row());
    if (role == Qt::DisplayRole) {
        return QString("%1").arg(g.name);
    }
    if (role == Qt::UserRole) return g.groupId;
    if (role == Qt::UserRole + 1) return g.name;
    return {};
}

QHash<int, QByteArray> GroupsModel::roleNames() const {
    QHash<int, QByteArray> rn = QAbstractListModel::roleNames();
    rn[Qt::UserRole] = "groupId";
    rn[Qt::UserRole + 1] = "groupName";
    return rn;
}

void GroupsModel::refreshFromDatabase() {
    beginResetModel();
    m_groups.clear();
    DatabaseManager &bd = DatabaseManager::instance();
    QVector<QVariantMap> groups = bd.listTeams();
    for (const auto &gmap : groups) {
        Group g;
        g.groupId = gmap.value("team_id").toInt();
        g.name = gmap.value("title").toString();
        // load members
        QVector<QVariantMap> members = bd.listTeamMembers(g.groupId);
        for (const auto &m : members) g.memberIds.append(m.value("user_id").toInt());
        m_groups.append(g);
    }
    endResetModel();
}

Group GroupsModel::groupAt(int row) const {
    if (row < 0 || row >= m_groups.count()) return {};
    return m_groups.at(row);
}

int GroupsModel::rowOfGroupId(int groupId) const {
    for (int i = 0; i < m_groups.size(); ++i)
        if (m_groups[i].groupId == groupId) return i;
    return -1;
}

int GroupsModel::addGroupToModel(const Group &g) {
    int row = m_groups.count();
    beginInsertRows(QModelIndex(), row, row);
    m_groups.append(g);
    endInsertRows();
    return row;
}

void GroupsModel::removeGroupAtRow(int row) {
    if (row < 0 || row >= m_groups.count()) return;
    beginRemoveRows(QModelIndex(), row, row);
    m_groups.removeAt(row);
    endRemoveRows();
}
