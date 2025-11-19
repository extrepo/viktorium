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
#include "databasemanager.h"

QuestionWidget::QuestionWidget(QString topic, qint64 quizId, qint64 questionId, QWidget *parent)
    : QWidget(parent), topic(topic), quizId(quizId), questionId(questionId)
{
    // Основной вертикальный layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *questionLayout = new QHBoxLayout();
    questionLayout->addWidget(new QLabel("Вопрос:"));
    questionLayout->addStretch();
    getLMButton = new QPushButton("Запросить у ИИ");
    questionLayout->addWidget(getLMButton);
    mainLayout->addLayout(questionLayout);

    // Поле для ввода вопроса
    questionEdit = new QLineEdit(this);
    questionEdit->setPlaceholderText("Введите вопрос");
    mainLayout->addWidget(questionEdit);

    //
    QHBoxLayout *difficultyLayout = new QHBoxLayout();
    difficultyComboBox = new QComboBox(this);
    difficultyComboBox->addItem("1", 1);
    difficultyComboBox->addItem("2", 2);
    difficultyComboBox->addItem("3", 3);
    difficultyLayout->addWidget(new QLabel("Сложность:"));
    difficultyLayout->addWidget(difficultyComboBox);
    difficultyLayout->addStretch();
    difficultyLayout->addWidget(new QLabel("Правильный ответ:"));
    rightAnswer = new QSpinBox();
    rightAnswer->setRange(1, 6);
    difficultyLayout->addWidget(rightAnswer);
    mainLayout->addLayout(difficultyLayout);

    // Список вариантов ответов
    QHBoxLayout *answersLayout = new QHBoxLayout();
    answersLayout->addWidget(new QLabel("Варианты ответа:"));
    answersLayout->addStretch();
    addAnswerButton = new QPushButton("Добавить");
    removeAnswerButton = new QPushButton("Удалить");
    answersLayout->addWidget(addAnswerButton);
    answersLayout->addWidget(removeAnswerButton);
    mainLayout->addLayout(answersLayout);
    answersList = new QListWidget(this);
    answersList->setEditTriggers(QAbstractItemView::DoubleClicked);
    mainLayout->addWidget(answersList);
    // Кнопка "Сохранить"
    QPushButton *saveButton = new QPushButton("Сохранить", this);
    mainLayout->addWidget(saveButton);

    if(questionId > 0) {
        DatabaseManager* db = &DatabaseManager::instance();
        auto result = db->getQuestion(questionId);
        questionEdit->setText(result["text"].toString());
        difficultyComboBox->setCurrentText(result["points"].toString());
        rightAnswer->setValue(result["answer"].toInt());
        answersList->clear();
        auto answers = db->listAnswersByQuestion(result["question_id"].toInt());
        for(auto& a : answers) {
            answersList->addItem(a["text"].toString());
        }
        for(int i=0;i<answersList->count();i++) {
            answersList->item(i)->setFlags(answersList->item(i)->flags() | Qt::ItemIsEditable);
        }
    }
    // Обработчики кнопок
    connect(addAnswerButton, &QPushButton::clicked, this, &QuestionWidget::onAddAnswerButton);
    connect(removeAnswerButton, &QPushButton::clicked, this, &QuestionWidget::onRemoveAnswerButton);
    // Обработчик кнопки сохранения
    connect(saveButton, &QPushButton::clicked, this, &QuestionWidget::onSaveButton);

    connect(getLMButton, &QPushButton::clicked, this, &QuestionWidget::onLMButton);
    connect(&lm, &LM::questionReady, this, &QuestionWidget::onLMReady);
    connect(&lm, &LM::errorOccurred, this, &QuestionWidget::onLMError);
}

void QuestionWidget::onSaveButton()
{
    if (questionEdit->text().size() < 5 || answersList->count() < 3) {
        QMessageBox::warning(nullptr, "Ошибка", "Заполните все поля.");
        return;
    }
    DatabaseManager* db = &DatabaseManager::instance();
    if(questionId > 0) {
        db->updateQuestion(questionId, quizId, questionEdit->text(), difficultyComboBox->currentIndex() + 1, rightAnswer->value());
    } else {
        db->addQuestion(quizId, questionEdit->text(), difficultyComboBox->currentIndex() + 1, rightAnswer->value(), questionId);
    }
    auto answers = db->listAnswersByQuestion(questionId);
    for(auto& a : answers) {
        db->removeAnswer(a["answer_id"].toInt());
    }
    for(int i=0;i<answersList->count();i++) {
        answersList->item(i);
        qint64 answerId;
        db->addAnswer(questionId, answersList->item(i)->text(), answerId);
    }
}

void QuestionWidget::onAddAnswerButton()
{
    answersList->addItem("Новый вариант ответа");
    for(int i=0;i<answersList->count();i++) {
        answersList->item(i)->setFlags(answersList->item(i)->flags() | Qt::ItemIsEditable);
    }
}

void QuestionWidget::onRemoveAnswerButton()
{
    int row = answersList->currentRow();
    if (row != -1) {
        delete answersList->takeItem(row);
    } else {
        QMessageBox::warning(nullptr, "Ошибка", "Выберите вариант для удаления.");
    }
}

void QuestionWidget::onLMButton()
{
    getLMButton->setEnabled(false);
    difficultyComboBox->setEnabled(false);
    questionEdit->setEnabled(false);
    answersList->setEnabled(false);
    rightAnswer->setEnabled(false);
    lm.requestQuestion(topic);
}

void QuestionWidget::onLMReady(const QVector<QVariant> &result)
{
    questionEdit->setText(result[0].toString());
    difficultyComboBox->setCurrentText(result[2].toString());
    answersList->clear();
    for(int i=3;i<result.size();i++) {
        answersList->addItem(result[i].toString());
    }
    getLMButton->setEnabled(true);
    difficultyComboBox->setEnabled(true);
    questionEdit->setEnabled(true);
    answersList->setEnabled(true);
    rightAnswer->setEnabled(true);
}

void QuestionWidget::onLMError(const QString &error)
{
    getLMButton->setEnabled(true);
    difficultyComboBox->setEnabled(true);
    questionEdit->setEnabled(true);
    answersList->setEnabled(true);
    QMessageBox::warning(nullptr, "Ошибка", "Ошибка генерации с помощью ИИ. Проверьте доступность LM Studio на этом компьютере");
}
