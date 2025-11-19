#ifndef QUIZPEWVIEWWIDGET_H
#define QUIZPEWVIEWWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>

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

    void onTableRowClicked(int index);
};

#endif
