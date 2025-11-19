#include "groupdialog.h"
#include "databasemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QSortFilterProxyModel>

GroupDialog::GroupDialog(QStandardItemModel* model, QWidget* parent)
    : QDialog(parent), m_peopleModel(model)
{
    setWindowTitle("Группа");
    m_nameEdit = new QLineEdit(this);
    m_search = new QLineEdit(this);
    m_search->setPlaceholderText("Поиск участников...");

    // DatabaseManager& bd = DatabaseManager::instance();
    // auto users = bd.listUsers();
    // for (const auto& user : users) {
    //     QStandardItem* item = new QStandardItem(user["surname"].toString() + " "
    //                                             + user["name"].toString() + " "
    //                                             + user["father_name"].toString());
    //     item->setData(user["usert_id"], Qt::UserRole);
    //     m_peopleModel->appendRow(item);
    // }


    m_availableView = new QListView(this);
    m_availableView->setModel(m_peopleModel);
    m_availableView->setSelectionMode(QAbstractItemView::SingleSelection);

    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setSourceModel(m_peopleModel);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setFilterRole(Qt::DisplayRole);
    m_availableView->setModel(m_proxy);

    m_membersView = new QListView(this);
    QStandardItemModel* membersModel = new QStandardItemModel(this);
    m_membersView->setModel(membersModel);
    m_membersView->setSelectionMode(QAbstractItemView::SingleSelection);

    m_addBtn = new QPushButton("Добавить ->", this);
    m_removeBtn = new QPushButton("Удалить", this);
    m_newPersonBtn = new QPushButton("Новый пользователь", this);

    QPushButton* ok = new QPushButton("OK", this);
    QPushButton* cancel = new QPushButton("Отмена", this);

    connect(m_search, &QLineEdit::textChanged, m_proxy, &QSortFilterProxyModel::setFilterFixedString);
    connect(m_addBtn, &QPushButton::clicked, this, &GroupDialog::onAddSelectedPerson);
    connect(m_removeBtn, &QPushButton::clicked, this, &GroupDialog::onRemoveSelectedMember);
    connect(m_newPersonBtn, &QPushButton::clicked, this, &GroupDialog::onAddNewPerson);
    connect(ok, &QPushButton::clicked, this, &GroupDialog::onAccept);
    connect(cancel, &QPushButton::clicked, this, &GroupDialog::reject);

    QVBoxLayout* left = new QVBoxLayout;
    left->addWidget(new QLabel("Доступные пользователи:"));
    left->addWidget(m_search);
    left->addWidget(m_availableView);
    left->addWidget(m_newPersonBtn);

    QVBoxLayout* middle = new QVBoxLayout;
    middle->addStretch();
    middle->addWidget(m_addBtn);
    middle->addWidget(m_removeBtn);
    middle->addStretch();

    QVBoxLayout* right = new QVBoxLayout;
    right->addWidget(new QLabel("Члены группы:"));
    right->addWidget(m_membersView);

    QHBoxLayout* top = new QHBoxLayout;
    top->addLayout(left, 3);
    top->addLayout(middle, 1);
    top->addLayout(right, 3);

    QFormLayout* form = new QFormLayout;
    form->addRow("Название группы:", m_nameEdit);

    QHBoxLayout* bottomButtons = new QHBoxLayout;
    bottomButtons->addStretch();
    bottomButtons->addWidget(ok);
    bottomButtons->addWidget(cancel);

    QVBoxLayout* main = new QVBoxLayout(this);
    main->addLayout(form);
    main->addLayout(top);
    main->addLayout(bottomButtons);

    setLayout(main);
}

void GroupDialog::setGroup(const Group& g) {
    m_groupId = g.groupId;
    m_nameEdit->setText(g.name);
    m_memberIds = g.memberIds;

    // fill membersView
    QStandardItemModel* m = qobject_cast<QStandardItemModel*>(m_membersView->model());
    m->clear();
    DatabaseManager &bd = DatabaseManager::instance();


    auto members = bd.listTeamUsers(m_groupId);
    qDebug() << members.size();
    for (const auto& member : members) {
        auto user = bd.getUser(member["user_id"].toInt());
        QStandardItem* it = new QStandardItem(user["surname"].toString() + " "
                                            + user["name"].toString() + " "
                                              + user["father_name"].toString());
        it->setData(member["user_id"].toInt(), Qt::UserRole);
        m->appendRow(it);
    }



    // for (int uid : m_memberIds) {
    //     QVariantMap user = bd.getUser(uid);
    //     QStandardItem* it = new QStandardItem(user.value("surname").toString() + " " + user.value("name").toString());
    //     it->setData(uid, Qt::UserRole);
    //     m->appendRow(it);
    // }
}

Group GroupDialog::resultGroup() const {
    Group g;
    g.groupId = m_groupId;
    g.name = m_nameEdit->text().trimmed();
    g.memberIds = m_memberIds;
    return g;
}

void GroupDialog::onAddSelectedPerson() {
    QModelIndex idx = m_availableView->currentIndex();
    if (!idx.isValid()) return;
    QModelIndex src = m_proxy->mapToSource(idx);
    int row = src.row();
    //Person p = m_peopleModel->personAt(row);

    int userId = m_peopleModel->data(src, Qt::UserRole).toInt();

    if (m_memberIds.contains(userId)) return;
    m_memberIds.append(userId);

    QStandardItemModel* m = qobject_cast<QStandardItemModel*>(m_membersView->model());
    QStandardItem* it = new QStandardItem(m_peopleModel->data(src, Qt::DisplayRole).toString());
    it->setData(userId, Qt::UserRole);
    m->appendRow(it);
}

void GroupDialog::onRemoveSelectedMember() {
    QModelIndex idx = m_membersView->currentIndex();
    if (!idx.isValid()) return;
    QStandardItemModel* m = qobject_cast<QStandardItemModel*>(m_membersView->model());
    QStandardItem* it = m->takeItem(idx.row());
    if (!it) return;
    int uid = it->data(Qt::UserRole).toInt();
    m_memberIds.removeAll(uid);
    delete it;
}

void GroupDialog::onAddNewPerson() {
    PersonDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Person p = dlg.person();
        qint64 outId;
        DatabaseManager &bd = DatabaseManager::instance();
        if (bd.addUser(p.surName, p.name, p.fatherName, outId)) {
            p.userId = (int)outId;
            // добавляем и в модель людей (чтобы сразу был видим)
            QStandardItem* item = new QStandardItem(p.displayName());
            item->setData(p.userId, Qt::UserRole);
            m_peopleModel->appendRow(item);

            // и сразу в членов группы
            m_memberIds.append(p.userId);
            QStandardItemModel* m = qobject_cast<QStandardItemModel*>(m_membersView->model());
            QStandardItem* it = new QStandardItem(p.displayName());
            it->setData(p.userId, Qt::UserRole);
            m->appendRow(it);
        }
    }
}

void GroupDialog::onAccept() {
    if (m_nameEdit->text().trimmed().isEmpty()) return;

    DatabaseManager &bd = DatabaseManager::instance();
    if (m_groupId < 0) {
        qint64 newG;
        if (!bd.addTeam(m_nameEdit->text().trimmed(), newG)) {
            reject();
            return;
        }
        m_groupId = (int)newG;
    } else {
        bd.updateTeam(m_groupId, m_nameEdit->text().trimmed());
    }

    QVector<QVariantMap> cur = bd.listTeamUsers(m_groupId);
    QSet<int> curSet;
    for (const auto &r : cur) curSet.insert(r.value("user_id").toInt());

    QSet<int> newSet = QSet<int>::fromList(m_memberIds);

    // удалить лишних
    for (int uid : curSet) {
        if (!newSet.contains(uid)) bd.removeTeamUser(uid, m_groupId);
    }
    // добавить новых
    for (int uid : newSet) {
        if (!curSet.contains(uid)) bd.addTeamUser(uid, m_groupId);
    }

    accept();
}
