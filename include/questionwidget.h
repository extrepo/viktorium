#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QSpinBox>
#include "lm.h"

class QuestionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QuestionWidget(QString topic, qint64 quizId, qint64 questionId, QWidget *parent = nullptr);
protected:
    QPushButton* getLMButton;
    QComboBox* difficultyComboBox;
    QLineEdit* questionEdit;
    QListWidget* answersList;
    LM lm;
    QString topic;
    QPushButton* addAnswerButton;
    QPushButton* removeAnswerButton;
    qint64 quizId;
    qint64 questionId;
    QSpinBox* rightAnswer;
   
private slots:
    void onSaveButton();
    void onAddAnswerButton();
    void onRemoveAnswerButton();
    void onLMButton();
    void onLMReady(const QVector<QVariant> &result);
    void onLMError(const QString &error);
    
};
