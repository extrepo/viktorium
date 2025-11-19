#include "quizpreviewwidget.h"
#include "participantsmodel.h"
#include "databasemanager.h"

#include <QVBoxLayout>

QuizPreViewWidget::QuizPreViewWidget(QWidget *parent): QWidget(parent)
{
    setObjectName("previewWidget");
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(5);
    setAttribute(Qt::WA_StyledBackground, true);

    title = new QLabel("Мероприятиe", this);
    title->setProperty("cssClass", "title");
    title->setAlignment(Qt::AlignLeft);
    title->setWordWrap(true);
    title->setFixedWidth(200);   // твоя фиксированная ширина

    QLabel* timerLabel = new QLabel("Таймер");
    timerLabel->setProperty("cssClass", "title2");

    timer = new QSpinBox();
    timer->setRange(5, 900);


    // connect(timer, &QSpinBox::valueChanged, this, [&](int value){

    // });

    layout->addWidget(title, 0, Qt::AlignLeft);
    layout->addWidget(timerLabel);
    layout->addWidget(timer);
    questionsWidget = new QuestionsWidget(this);
    layout->addWidget(questionsWidget);
}

void QuizPreViewWidget::onTableRowClicked(int index)
{
    DatabaseManager& db = DatabaseManager::instance();

    QVariantMap event = db.getQuiz(index);

    QLocale ru(QLocale::Russian);

    title->setText(event["topic"].toString());
    timer->setValue(event["timer"].toInt());

    questionsWidget->showQuizData(event["topic"].toString(), index);

}
