#include "previewwidget.h"
#include "participantsmodel.h"
#include "databasemanager.h""

#include <QVBoxLayout>

PreViewWidget::PreViewWidget(QWidget *parent): QWidget(parent)
{
    setObjectName("previewWidget");
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(5);
    setAttribute(Qt::WA_StyledBackground, true);

    QWidget* titleContainer = new QWidget(this);
    titleContainer->setProperty("cssClass", "container");
    QHBoxLayout* titleLayout = new QHBoxLayout(titleContainer);
    titleLayout->setContentsMargins(0, 0, 0, 0);

    title = new QLabel("Мероприятиe", this);
    title->setProperty("cssClass", "title");
    title->setAlignment(Qt::AlignCenter);
    type = new QLabel("Индивидуальный", this);
    type->setProperty("cssClass", "subtitle");
    type->setAlignment(Qt::AlignCenter);

    date = new QLabel();
    date->setProperty("cssClass", "date");
    time = new QLabel();
    time->setProperty("cssClass", "time");

    titleLayout->addWidget(title, 0, Qt::AlignRight | Qt::AlignVCenter);
    titleLayout->addWidget(type, 0, Qt::AlignRight | Qt::AlignVCenter);
    titleLayout->addStretch();

    QLabel* quizLabel = new QLabel("Квиз");
    quizLabel->setProperty("cssClass", "title2");

    quizComboBox = new QComboBox();

    QLabel* participantsLabel = new QLabel("Участники");
    participantsLabel->setProperty("cssClass", "title2");

    participantSelectorWidget = new ParticipantSelectorWidget(this);

    // Добавить участников из базы
    //    participantsWidget->addExistingParticipant({"Иван", "Иванов", "Иванович"});
    //    participantsWidget->addExistingParticipant({"Петр", "Петров", ""});


    QLabel* settingsLabel = new QLabel("Настройки");
    settingsLabel->setProperty("cssClass", "title2");

    layout->addWidget(titleContainer);
    layout->addWidget(date);
    layout->addWidget(time);
    layout->addWidget(quizLabel);
    layout->addWidget(quizComboBox);
    layout->addWidget(participantsLabel);
    layout->addWidget(participantSelectorWidget);
    layout->addWidget(settingsLabel);
    layout->addStretch();
}

void PreViewWidget::onTableRowClicked(int index)
{
    participantSelectorWidget->update(index);

    DatabaseManager& db = DatabaseManager::instance();

    QVariantMap event = db.getEvent(index);

    QLocale ru(QLocale::Russian);

    title->setText(event["title"].toString());
    type->setText(event["type"].toInt() == 0 ? "Индивидуальный" : "Командный");
    QDateTime dateTime = QDateTime::fromSecsSinceEpoch(event["time"].toLongLong());
    date->setText(ru.toString(dateTime.date(), "d MMMM yyyy"));
    time->setText(dateTime.time().toString("HH:mm"));

    //QVariantMap quiz = db.getQuiz(event["quiz_id"].toInt());
    QVector<QVariantMap> quizes = db.listQuizzes();

    QStandardItemModel *model = new QStandardItemModel(this);
    for (const auto&q : quizes) {
        QStandardItem *item = new QStandardItem(q["topic"].toString());
        item->setData(q["quiz_id"].toInt(), Qt::UserRole);        // userData сюда
        model->appendRow(item);
    }

    QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy->setFilterRole(Qt::DisplayRole); // фильтруем по названию
    proxy->setFilterKeyColumn(0);
    proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxy->sort(0, Qt::AscendingOrder);

    quizComboBox->setModel(proxy);
    quizComboBox->setCurrentIndex(quizComboBox->findData(event["quiz_id"].toInt(), Qt::UserRole));

}
