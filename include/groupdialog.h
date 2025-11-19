#pragma once
#include <QDialog>
#include "participantsmodel.h"
#include "groupmodel.h"

#include <QStandardItemModel>

class QLineEdit;
class QListView;
class QPushButton;
class QSortFilterProxyModel;

class GroupDialog : public QDialog {
    Q_OBJECT
public:
    // editing existing group: pass group (groupId >=0) and models
    explicit GroupDialog(QStandardItemModel* model, QWidget* parent = nullptr);
    void setGroup(const Group& g); // set for edit
    Group resultGroup() const;     // group created/edited

private slots:
    void onAddSelectedPerson();
    void onRemoveSelectedMember();
    void onAddNewPerson();
    void onAccept();

private:

    QStandardItemModel* m_peopleModel;

    QLineEdit* m_nameEdit;
    QLineEdit* m_search;
    QListView* m_availableView;
    QListView* m_membersView;
    QSortFilterProxyModel* m_proxy;

    QPushButton* m_addBtn;
    QPushButton* m_removeBtn;
    QPushButton* m_newPersonBtn;

    // local buffer of member userIds
    QList<int> m_memberIds;
    int m_groupId = -1; // -1 -> new
};
