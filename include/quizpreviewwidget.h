#ifndef QUIZPEWVIEWWIDGET_H
#define QUIZPEWVIEWWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include "questionswidget.h"

class QuizPreViewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QuizPreViewWidget(QWidget *parent = nullptr);

    QLabel* title;
    QSpinBox* timer;

    QSpinBox* questionNumber;

    QPushButton* generateButton;

    QPushButton* editButton;
    QPushButton* deleteButton;

    QuestionsWidget* questionsWidget;

    void onTableRowClicked(int index);
};

#endif
