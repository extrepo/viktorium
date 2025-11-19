#include "createquizdialog.h"
#include <QDebug>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QCompleter>

CreateQuizDialog::CreateQuizDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Новый квиз");
    setModal(true);
    setMinimumWidth(420);

    titleEdit = new QLineEdit(this);

    timerSpinBox = new QSpinBox(this);
    timerSpinBox->setRange(5, 900);
    timerSpinBox->setValue(60);

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

    addRow("Тема", titleEdit);
    addRow("Таймер", timerSpinBox);

    QHBoxLayout *btns = new QHBoxLayout();
    btns->addStretch();
    btns->addWidget(btnCancel);
    btns->addWidget(btnCreate);
    btns->setSpacing(8);

    main->addSpacing(6);
    main->addLayout(btns);

    connect(btnCreate, &QPushButton::clicked, this, &CreateQuizDialog::onCreateClicked);
    connect(btnCancel,  &QPushButton::clicked, this, &CreateQuizDialog::onCancelClicked);
}

void CreateQuizDialog::onCreateClicked()
{
    QString title = titleEdit->text().trimmed();
    if (title.isEmpty()) {
        titleEdit->setFocus();
        return;
    }

    Quiz ev;
    ev.id = -1;
    ev.topic = title;
    ev.timer = timerSpinBox->value();

    emit eventCreated(ev);
    accept();
}

void CreateQuizDialog::onCancelClicked()
{
    reject();
}
