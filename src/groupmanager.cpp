#include "groupmanager.h"
#include "groupdialog.h"
#include "databasemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListView>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QSortFilterProxyModel>

GroupManagerWidget::GroupManagerWidget(QWidget* parent)
    : QWidget(parent)
{
    m_peopleModel = new QStandardItemModel(this);
    DatabaseManager& db = DatabaseManager::instance();
    auto users = db.listUsers();
    for (const auto& user : users) {
        QStandardItem* item = new QStandardItem(user["surname"].toString() + " "
                                                + user["name"].toString() + " "
                                                + user["father_name"].toString());
        item->setData(user["user_id"].toInt(), Qt::UserRole);
        m_peopleModel->appendRow(item);
    }

    m_groupsModel = new GroupsModel(this);

    m_searchGroups = new QLineEdit(this);
    m_searchGroups->setPlaceholderText("Поиск команды...");

    m_groupsView = new QListView(this);
    m_groupsView->setSelectionMode(QAbstractItemView::SingleSelection);

    m_groupsProxy = new QSortFilterProxyModel(this);
    m_groupsProxy->setSourceModel(m_groupsModel);
    m_groupsProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_groupsProxy->setFilterRole(Qt::DisplayRole);
    m_groupsView->setModel(m_groupsProxy);

    m_createBtn = new QPushButton("Новая команда", this);
    m_editBtn = new QPushButton("Редактировать", this);
    m_deleteBtn = new QPushButton("Удалить", this);

    QHBoxLayout* topButtons = new QHBoxLayout;
    topButtons->addWidget(m_createBtn);
    topButtons->addWidget(m_editBtn);
    topButtons->addWidget(m_deleteBtn);
    topButtons->addStretch();

    QVBoxLayout* main = new QVBoxLayout(this);
    QLabel* comandTitle = new QLabel("Команды");
    comandTitle->setProperty("cssClass", "subtitle");
    main->addWidget(comandTitle);
    main->addWidget(m_searchGroups);
    main->addWidget(m_groupsView);
    main->addLayout(topButtons);
    setLayout(main);

    connect(m_searchGroups, &QLineEdit::textChanged, m_groupsProxy, &QSortFilterProxyModel::setFilterFixedString);
    connect(m_createBtn, &QPushButton::clicked, this, &GroupManagerWidget::onCreateGroup);
    connect(m_editBtn, &QPushButton::clicked, this, &GroupManagerWidget::onEditGroup);
    connect(m_deleteBtn, &QPushButton::clicked, this, &GroupManagerWidget::onDeleteGroup);
}

void GroupManagerWidget::onCreateGroup() {
    DatabaseManager& bd = DatabaseManager::instance();
    auto users = bd.listUsers();
    GroupDialog dlg(m_peopleModel, this);
    if (dlg.exec() == QDialog::Accepted) {
        Group g = dlg.resultGroup();
        // Уже добавлен в БД внутри dialog (см. onAccept). Просто обновим модель.
        m_groupsModel->refreshFromDatabase();
    }
}

void GroupManagerWidget::onEditGroup() {
    QModelIndex idx = m_groupsView->currentIndex();
    if (!idx.isValid()) return;
    QModelIndex src = m_groupsProxy->mapToSource(idx);
    Group g = m_groupsModel->groupAt(src.row());

    GroupDialog dlg(m_peopleModel, this);
    dlg.setGroup(g);
    if (dlg.exec() == QDialog::Accepted) {
        // изменения уже применены в БД в dialog::onAccept
        m_groupsModel->refreshFromDatabase();
    }
}

void GroupManagerWidget::onDeleteGroup() {
    QModelIndex idx = m_groupsView->currentIndex();
    if (!idx.isValid()) return;
    QModelIndex src = m_groupsProxy->mapToSource(idx);
    Group g = m_groupsModel->groupAt(src.row());
    auto ret = QMessageBox::question(this, "Удалить группу", QString("Удалить группу \"%1\"?").arg(g.name));
    if (ret != QMessageBox::Yes) return;
    DatabaseManager &bd = DatabaseManager::instance();
    if (bd.removeTeam(g.groupId)) {
        m_groupsModel->refreshFromDatabase();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось удалить группу (БД).");
    }
}
