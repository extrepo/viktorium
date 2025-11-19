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
#include <QScrollArea>
#include "lm.h"


class QuestionsWidget : public QScrollArea
{
    Q_OBJECT

public:
    explicit QuestionsWidget(QWidget *parent = nullptr);
    void showQuizData(QString topic, qint64 quizId);
protected:
    QString topic;
    QPushButton* addQuestionButton;
    QPushButton* removeAnswerButton;
    qint64 quizId;
    QVector<QWidget*> questions;
    QVBoxLayout *mainLayout;
   
private slots:
    void onAddQuestion();
};
