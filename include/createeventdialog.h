#ifndef CREATEEVENTDIALOG_H
#define CREATEEVENTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include "eventsmodel.h"

struct QuizItem {
    int id;
    QString title;
};

class CreateEventDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateEventDialog(const QList<QuizItem> &quizzes, QWidget *parent = nullptr);

signals:
    void eventCreated(const Event &ev, int quizId);

private slots:
    void onCreateClicked();
    void onCancelClicked();

private:
    QLineEdit *titleEdit;
    QDateEdit *dateEdit;
    QComboBox* hour;
    QComboBox* minute;
    QComboBox *typeCombo;
    QComboBox *quizCombo;

    QPushButton *btnCreate;
    QPushButton *btnCancel;
};

#endif // CREATEEVENTDIALOG_H
