#ifndef PEWVIEWWIDGET_H
#define PEWVIEWWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include "groupmanager.h"

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

    QPushButton* generateButton;

    QComboBox* quizComboBox;
    ParticipantSelectorWidget *participantSelectorWidget;
    GroupManagerWidget* groupmanagetWidget;

    void onTableRowClicked(int index);

    int eventId = -1;
};

#endif
