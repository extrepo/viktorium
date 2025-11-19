#ifndef CREATEQUIZDIALOG_H
#define CREATEQUIZDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>

#include "quizmodel.h"

class CreateQuizDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateQuizDialog(QWidget *parent = nullptr);

signals:
    void eventCreated(const Quiz &ev);

private slots:
    void onCreateClicked();
    void onCancelClicked();

private:
    QLineEdit *titleEdit;
    QSpinBox *timerSpinBox;

    QPushButton *btnCreate;
    QPushButton *btnCancel;
};

#endif // CREATEEVENTDIALOG_H
