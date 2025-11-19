#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "eventsmodel.h"


#include <QLabel>
#include <QMainWindow>
#include <QFrame>
#include <QTableView>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QComboBox>

class ParticipantSelectorWidget;

class HeaderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HeaderWidget(QWidget *parent = nullptr);
};

class SidebarWidget : public QFrame
{
    Q_OBJECT
public:
    explicit SidebarWidget(QWidget *parent = nullptr);

    // Установить индекс активного пункта (0..count-1). Если индекс вне диапазона — ничего не делает.
    void setActiveIndex(int index);
    int activeIndex() const { return m_activeIndex; }

    // Получить количество пунктов
    int count() const { return m_buttons.size(); }

signals:
    void activeIndexChanged(int newIndex);

private:
    void initUi();
    QPushButton* createSidebarButton(const QString &text, const QString &iconPath);

private:
    QVector<QPushButton*> m_buttons;
    QButtonGroup *m_group = nullptr;
    int m_activeIndex = -1;
};

class PreViewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PreViewWidget(QWidget *parent = nullptr);

    QLabel* title;
    QLabel* type;
    QLabel* date;
    QLabel* time;

    QPushButton* editButton;
    QPushButton* deleteButton;

    QComboBox* quizComboBox;
    ParticipantSelectorWidget *participantSelectorWidget;

    void onTableRowClicked(int index);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QWidget *central;
    HeaderWidget *header;
    SidebarWidget *sidebar;
    PreViewWidget* preview;

    QPushButton *btnOpen;
    QPushButton *btnEdit;
    QPushButton *btnExport;

    QTableView *tableView;
    EventsModel *eventsModel;

    QWidget* createEventWidget();
    QWidget* createQuizWidget();

    QSortFilterProxyModel *proxyModel;

    QPushButton* addEventButton;

private slots:
    void onAddEventButtonClicked();
};

#endif // MAINWINDOW_H
