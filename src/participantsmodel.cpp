#include "participantsmodel.h"
#include "databasemanager.h"
#include <QLayout>
#include <QStringListModel>
#include <QCompleter>
#include <QFormLayout>
#include <QStandardItemModel>
#include <QLabel>

ParticipantsModel::ParticipantsModel(QObject* parent)
: QAbstractListModel(parent)
{
}


int ParticipantsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_parts.count();
}


QVariant ParticipantsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return {};
    if (index.row() < 0 || index.row() >= m_parts.size()) return {};
    const Person &p = m_parts.at(index.row());
    if (role == Qt::DisplayRole) return p.displayName();
    return {};
}


void ParticipantsModel::addParticipant(const Person &p)
{
    for (const Person &existing : m_parts) {
        if (existing.surName == p.surName && existing.name == p.name && existing.fatherName == p.fatherName)
        return;
    }
    beginInsertRows(QModelIndex(), m_parts.count(), m_parts.count());
    m_parts.append(p);
    endInsertRows();
    emit participantsChanged();
}


void ParticipantsModel::removeParticipantAt(int row)
{
    if (row < 0 || row >= m_parts.count()) return;
    beginRemoveRows(QModelIndex(), row, row);
    m_parts.removeAt(row);
    endRemoveRows();
    emit participantsChanged();
}

PersonDialog::PersonDialog(QWidget* parent)
: QDialog(parent)
{
    setWindowTitle("Новый человек");
    firstEdit = new QLineEdit(this);
    lastEdit = new QLineEdit(this);
    middleEdit = new QLineEdit(this);


    okButton = new QPushButton("OK", this);
    cancelButton = new QPushButton("Отмена", this);


    connect(okButton, &QPushButton::clicked, this, &PersonDialog::onAccept);
    connect(cancelButton, &QPushButton::clicked, this, &PersonDialog::reject);


    QFormLayout* form = new QFormLayout;
    form->addRow("Фамилия:", firstEdit);
    form->addRow("Имя:", lastEdit);
    form->addRow("Отчество:", middleEdit);


    QHBoxLayout* buttons = new QHBoxLayout;
    buttons->addStretch();
    buttons->addWidget(okButton);
    buttons->addWidget(cancelButton);


    QVBoxLayout* main = new QVBoxLayout(this);
    main->addLayout(form);
    main->addLayout(buttons);


    setLayout(main);
}


void PersonDialog::setPerson(const Person &p)
{
    firstEdit->setText(p.surName);
    lastEdit->setText(p.name);
    middleEdit->setText(p.fatherName);
}


Person PersonDialog::person() const
{
    Person p;
    p.surName = firstEdit->text().trimmed();
    p.name = lastEdit->text().trimmed();
    p.fatherName = middleEdit->text().trimmed();
    return p;
}


void PersonDialog::onAccept()
{
    // basic validation: at least one of name/last should be provided
    if (firstEdit->text().trimmed().isEmpty() && lastEdit->text().trimmed().isEmpty()) {
    // simple feedback - disable accept
    // In production you might show QMessageBox
    return;
    }
    accept();
}

ParticipantSelectorWidget::ParticipantSelectorWidget(QWidget* parent)
: QWidget(parent)
    {
    DatabaseManager& bd = DatabaseManager::instance();

    m_participantsModel = new ParticipantsModel(this);


    m_search = new QLineEdit(this);
    m_search->setPlaceholderText("Поиск...");


    m_peopleView = new QListView(this);
    m_peopleView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_peopleView->setSelectionMode(QAbstractItemView::SingleSelection);


    m_peopleModel = new QStandardItemModel();
    QVector<QVariantMap> users = bd.listUsers();
    for (const auto& user : users) {
        QStandardItem *item = new QStandardItem(user["surname"].toString() + " "
                                              + user["name"].toString() + " "
                                              + user["father_name"].toString());
        item->setData(user["user_id"].toInt(), Qt::UserRole);        // userData сюда
        m_peopleModel->appendRow(item);
    }


    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setSourceModel(m_peopleModel);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setFilterRole(Qt::DisplayRole);


    m_peopleView->setModel(m_proxy);


    m_participantsView = new QListView(this);
    m_participantsView->setModel(m_participantsModel);
    m_participantsView->setEditTriggers(QAbstractItemView::NoEditTriggers);


    m_addNewBtn = new QPushButton("Добавить нового", this);


    // Layout
    QVBoxLayout* left = new QVBoxLayout;
    QLabel* leftLabel = new QLabel("Существующие");
    left->addWidget(leftLabel);
    leftLabel->setProperty("cssClass", "subtitle");
    left->addWidget(m_search);
    left->addWidget(m_peopleView);
    left->addWidget(m_addNewBtn);


    QVBoxLayout* right = new QVBoxLayout;
    QLabel* rightLabel = new QLabel("Участники мероприятия");
    right->addWidget(rightLabel);
    rightLabel->setProperty("cssClass", "subtitle");
    right->setProperty("cssClass", "subtitle");
    right->addWidget(m_participantsView);


    QHBoxLayout* main = new QHBoxLayout(this);
    main->addLayout(left, 1);
    main->addLayout(right, 1);


    setLayout(main);


    // connections
    connect(m_search, &QLineEdit::textChanged, m_proxy, &QSortFilterProxyModel::setFilterFixedString);
    connect(m_addNewBtn, &QPushButton::clicked, this, &ParticipantSelectorWidget::onAddNew);
    connect(m_peopleView, &QListView::activated, this, &ParticipantSelectorWidget::onPeopleActivated);
    connect(m_participantsView, &QListView::activated, this, &ParticipantSelectorWidget::onParticipantActivated);
    connect(m_participantsModel, &ParticipantsModel::participantsChanged, this, &ParticipantSelectorWidget::onParticipantsChanged);
}

QList<Person> ParticipantSelectorWidget::selectedParticipants() const
{
    return m_participantsModel->participants();
}

void ParticipantSelectorWidget::update(int eventId)
{
    this->eventId = eventId;

    DatabaseManager& bd = DatabaseManager::instance();

    QVector<QVariantMap> parts = bd.listParticipantsByEvent(eventId);

    m_participantsModel = new ParticipantsModel();

    m_participantsView->reset();

    for (const auto& part : parts) {
        QVariantMap user = bd.getUser(part["user_id"].toInt());
        Person p{user["surname"].toString(), user["name"].toString(), user["father_name"].toString()};
        m_participantsModel->addParticipant(p);
    }
}


void ParticipantSelectorWidget::onAddNew()
{
    DatabaseManager& bd = DatabaseManager::instance();

    PersonDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Person p = dlg.person();

        qint64 id;
        bd.addUser(p.surName, p.name, p.fatherName, id);

        // add to people model
        QStandardItem *item = new QStandardItem(p.displayName());
        item->setData(id, Qt::UserRole);        // userData сюда
        m_peopleModel->appendRow(item);


        // and add to participants
        m_participantsModel->addParticipant(p);

        // optional: for usability, clear search and select newly added person
        m_search->clear();
    }
}


void ParticipantSelectorWidget::onPeopleActivated(const QModelIndex &index)
{
    DatabaseManager& bd = DatabaseManager::instance();

    if (!index.isValid()) return;
    QModelIndex src = m_proxy->mapToSource(index);
    QVariantMap user = bd.getUser(src.data(Qt::UserRole).toInt());
    //Person p = m_peopleModel->personAt(src.row());

    Person p{user["surname"].toString(), user["name"].toString(), user["father_name"].toString()};

    m_participantsModel->addParticipant(p);

    int count = bd.listParticipantsByEvent(eventId).size();

    qint64 id;
    qDebug() << eventId << src.data(Qt::UserRole).toInt() << count;
    bd.addParticipant(eventId, src.data(Qt::UserRole).toInt(), 0, count, id);
}


void ParticipantSelectorWidget::onParticipantActivated(const QModelIndex &index)
{
    if (!index.isValid()) return;
    // remove on activation (double click)
    m_participantsModel->removeParticipantAt(index.row());
}


void ParticipantSelectorWidget::onParticipantsChanged()
{
    emit participantsChanged(m_participantsModel->participants());
}
