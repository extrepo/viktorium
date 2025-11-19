#include "createeventdialog.h"
#include <QDebug>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QCompleter>

CreateEventDialog::CreateEventDialog(const QList<QuizItem> &quizzes, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Новое событие");
    setModal(true);
    setMinimumWidth(420);

    titleEdit = new QLineEdit(this);

    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("dd.MM.yyyy");

    hour = new QComboBox;
    for (int i = 0; i < 24; ++i)
        hour->addItem(QString("%1").arg(i, 2, 10, QLatin1Char('0')));

    minute = new QComboBox;
    for (int i = 0; i < 60; i += 5)
        minute->addItem(QString("%1").arg(i, 2, 10, QLatin1Char('0')));

    typeCombo = new QComboBox(this);
    typeCombo->addItem("Индивидуальный", 0);
    typeCombo->addItem("Командный", 1);

    quizCombo = new QComboBox(this);
    quizCombo->setEditable(true);

    QStandardItemModel *model = new QStandardItemModel(this);
    for (const QuizItem &q : quizzes) {
        QStandardItem *item = new QStandardItem(q.title);
        item->setData(q.id, Qt::UserRole);        // userData сюда
        model->appendRow(item);
    }

    QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy->setFilterRole(Qt::DisplayRole); // фильтруем по названию
    proxy->setFilterKeyColumn(0);
    proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxy->sort(0, Qt::AscendingOrder);


    quizCombo->setModel(proxy);

    btnCreate = new QPushButton("Создать");
    btnCreate->setObjectName("CreateButton");      // применит синий стиль

    btnCancel = new QPushButton("Отмена");

    QVBoxLayout *main = new QVBoxLayout(this);

    auto addRow = [&](const QString &label, QWidget *field) {
        QVBoxLayout *row = new QVBoxLayout();
        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet("font-weight: 600; margin-bottom: 4px;");
        row->addWidget(lbl);
        row->addWidget(field);
        main->addLayout(row);
    };

    QWidget* timeEdit = new QWidget(this);
    timeEdit->setProperty("cssClass", "container");
    QHBoxLayout* timeEditLayout = new QHBoxLayout(timeEdit);
    timeEditLayout->setContentsMargins(0, 0, 0, 0);
    timeEditLayout->setSpacing(0);
    timeEditLayout->addWidget(hour);
    timeEditLayout->addWidget(minute);
    timeEditLayout->addStretch();

    addRow("Название события", titleEdit);
    addRow("Дата", dateEdit);
    addRow("Время", timeEdit);
    addRow("Тип", typeCombo);
    addRow("Квиз", quizCombo);

    QHBoxLayout *btns = new QHBoxLayout();
    btns->addStretch();
    btns->addWidget(btnCancel);
    btns->addWidget(btnCreate);
    btns->setSpacing(8);

    main->addSpacing(6);
    main->addLayout(btns);

    connect(btnCreate, &QPushButton::clicked, this, &CreateEventDialog::onCreateClicked);
    connect(btnCancel,  &QPushButton::clicked, this, &CreateEventDialog::onCancelClicked);
}

void CreateEventDialog::onCreateClicked()
{
    QString title = titleEdit->text().trimmed();
    if (title.isEmpty()) {
        titleEdit->setFocus();
        return;
    }

    Event ev;
    ev.id = -1;
    ev.title = title;
    ev.date = QDateTime(dateEdit->date(), QTime(hour->currentText().toInt(), minute->currentText().toInt())); //dateTimeEdit->dateTime();
    ev.type = typeCombo->currentData().toInt();

    int quizId = quizCombo->currentData().toInt();

    emit eventCreated(ev, quizId);
    accept();
}

void CreateEventDialog::onCancelClicked()
{
    reject();
}
