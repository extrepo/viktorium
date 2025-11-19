#pragma once
#include <QAbstractListModel>
#include <QList>

struct Group {
    int groupId = -1;
    QString name;
    QList<int> memberIds; // user IDs
};

class GroupsModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit GroupsModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void refreshFromDatabase();
    Group groupAt(int row) const;
    int rowOfGroupId(int groupId) const;

    int addGroupToModel(const Group &g); // append, returns row
    void removeGroupAtRow(int row);

private:
    QList<Group> m_groups;
};
