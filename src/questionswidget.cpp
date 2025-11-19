#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include "questionwidget.h"
#include "questionswidget.h"
#include "databasemanager.h"

QuestionsWidget::QuestionsWidget(QWidget *parent)
    : QScrollArea(parent)
{
    setWidgetResizable(true);

    QWidget* w = new QWidget();
    mainLayout = new QVBoxLayout(w);
    addQuestionButton = new QPushButton("Добавить вопрос");
    addQuestionButton->setVisible(false);
    mainLayout->addWidget(addQuestionButton);
    setWidget(w);
    // Обработчики кнопок
    connect(addQuestionButton, &QPushButton::clicked, this, &QuestionsWidget::onAddQuestion);
}

void QuestionsWidget::onAddQuestion()
{
    QuestionWidget* w = new QuestionWidget(topic, quizId, 0, this);
    mainLayout->addWidget(w);
    questions.push_back(w);
}

void QuestionsWidget::showQuizData(QString topic, qint64 quizId)
{
    this->topic = topic;
    this->quizId = quizId;
    for(auto& q : questions) delete q;
    questions.clear();
    DatabaseManager* db = &DatabaseManager::instance();
    auto qst = db->listQuestionsByQuiz(quizId);
    for(auto& q : qst) {
        QuestionWidget* w = new QuestionWidget(topic, quizId, q["question_id"].toInt(), this);
        mainLayout->addWidget(w);
        questions.push_back(w);
    }
    addQuestionButton->setVisible(true);
}