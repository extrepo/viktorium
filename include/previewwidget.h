#ifndef PEWVIEWWIDGET_H
#define PEWVIEWWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>

class ParticipantSelectorWidget;

class PreViewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PreViewWidget(QWidget *parent = nullptr);

    QLabel* title;
    QLabel* type;
    QLabel* date;
    QLabel* time;

    QComboBox* quizComboBox;
    ParticipantSelectorWidget *participantSelectorWidget;

    void onTableRowClicked(int index);
};

#endif
