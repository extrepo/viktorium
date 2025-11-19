#pragma once
#include <QWidget>
#include "participantsmodel.h"
#include "groupmodel.h"

class QListView;
class QPushButton;
class QLineEdit;
class QSortFilterProxyModel;

class GroupManagerWidget : public QWidget {
    Q_OBJECT
public:
    explicit GroupManagerWidget(QWidget* parent = nullptr);

private slots:
    void onCreateGroup();
    void onEditGroup();
    void onDeleteGroup();

private:
    QStandardItemModel* m_peopleModel;
    GroupsModel* m_groupsModel;

    QLineEdit* m_searchGroups;
    QListView* m_groupsView;
    QPushButton* m_createBtn;
    QPushButton* m_editBtn;
    QPushButton* m_deleteBtn;
    QSortFilterProxyModel* m_groupsProxy;
};
