#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "previewwidget.h"
#include "quizpreviewwidget.h"
#include "eventsmodel.h"
#include "quizmodel.h"


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
    QuizPreViewWidget* quizPreview;


    QPushButton *btnOpen;
    QPushButton *btnEdit;
    QPushButton *btnExport;

    QTableView *tableView;
    QTableView *quizView;
    EventsModel *eventsModel;
    QuizModel *quizModel;

    QWidget* createEventWidget();
    QWidget* createQuizWidget();

    QSortFilterProxyModel *proxyModel;
    QSortFilterProxyModel *quizProxyModel;

    QPushButton* addEventButton;
    QPushButton* addQuizButton;

private slots:
    void onAddEventButtonClicked();
    void onAddQuizButtonClicked();
};

#endif // MAINWINDOW_H
