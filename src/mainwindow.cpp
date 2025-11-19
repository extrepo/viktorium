#include "mainwindow.h"
#include "databasemanager.h"
#include "createeventdialog.h"
#include "createquizdialog.h"
#include "reporthelper.h"


#include <QHeaderView>
#include <QFile>
#include <QApplication>
#include <QDebug>
#include <QButtonGroup>
#include <QStandardItemModel>


static QString iconPathOrFallback(const QString &path) {
    // если файл существует — возвращаем как есть, иначе возвращаем empty (qt покажет без иконки)
    if (QFile::exists(path)) return path;
    return QString();
}

// ============================
// HeaderWidget
// ============================
HeaderWidget::HeaderWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("headerWidget");
    setFixedHeight(50);
    setAttribute(Qt::WA_StyledBackground, true);

    //setStyleSheet("HeaderWidget {backgroud: white; border: 5px;} ");

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 0, 16, 0);

    QLabel *title = new QLabel("Viktorium");
    layout->addWidget(title);
    layout->addStretch();
}

// ============================
// SidebarWidget
// ============================
SidebarWidget::SidebarWidget(QWidget *parent)
    : QFrame(parent)
{
    setMinimumWidth(200);
    setObjectName("sidebarWidget");
    setFrameShape(QFrame::NoFrame);
    initUi();
}

void SidebarWidget::initUi()
{
    // вертикальный layout для кнопок
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(4);

    // создаём группу, чтобы легко управлять эксклюзивностью
    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);

    // Создаём кнопки
    // Здесь используются относительные пути к иконкам; при необходимости замените на ресурсы.
    // Имена иконок можно положить в assets/icons/...
    struct Item { QString text; QString icon; };
    QVector<Item> items = {
        { QStringLiteral("Мероприятия"),   QStringLiteral("assets/ic_events.svg") },
        { QStringLiteral("Квизы"),         QStringLiteral("assets/ic_quiz.svg") },
        { QStringLiteral("Статистика"),     QStringLiteral("assets/ic_settings.svg") }
    };

    const QSize iconSize(20, 20);
    const int btnHeight = 44; // одинаковая высота кнопок

    for (int i = 0; i < items.size(); ++i) {
        QPushButton *btn = createSidebarButton(items[i].text, iconPathOrFallback(items[i].icon));
        btn->setCheckable(true);
        btn->setMinimumHeight(btnHeight);
        btn->setMaximumHeight(btnHeight);
        btn->setIconSize(iconSize);
        btn->setProperty("sidebarIndex", i); // удобно для отладки/стилей
        m_buttons.append(btn);
        layout->addWidget(btn);
        m_group->addButton(btn, i);

        // при клике обновляем activeIndex через слот группы
    }

    layout->addStretch();

    // подключаем сигнал от группы к нашей обработке
    connect(m_group, QOverload<int,bool>::of(&QButtonGroup::buttonToggled), this,
            [this](int id, bool checked){
                if (checked) {
                    setActiveIndex(id);
                } else {
                    // если кнопка была снята и это не та же самая — игнорируем
                }
            });

    // По умолчанию активируем первый пункт (если есть)
    if (!m_buttons.isEmpty()) {
        setActiveIndex(0);
        m_buttons[0]->setChecked(true);
    }
}

QPushButton* SidebarWidget::createSidebarButton(const QString &text, const QString &iconPath)
{
    QPushButton *btn = new QPushButton(this);
    btn->setText(text);
    btn->setObjectName("sidebarButton");
    btn->setFlat(true); // стилизуем через QSS
    btn->setCursor(Qt::PointingHandCursor);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    if (!iconPath.isEmpty()) {
        QIcon ico(iconPath);
        btn->setIcon(ico);
        // небольшой отступ между иконкой и текстом
        btn->setStyleSheet("QPushButton#sidebarButton { text-align: left; padding-left: 8px; }");
    } else {
        // если иконки нет, оставляем только текст; можно задать отступ для визуального баланса
        btn->setStyleSheet("QPushButton#sidebarButton { text-align: left; padding-left: 16px; }");
    }

    return btn;
}

void SidebarWidget::setActiveIndex(int index)
{
    if (index < 0 || index >= m_buttons.size()) return;
    if (m_activeIndex == index) return;

    // Снимаем свойство active с прежней кнопки
    if (m_activeIndex >= 0 && m_activeIndex < m_buttons.size()) {
        QPushButton *oldBtn = m_buttons[m_activeIndex];
        oldBtn->setProperty("active", false);
        oldBtn->style()->unpolish(oldBtn);
        oldBtn->style()->polish(oldBtn);
        oldBtn->update();
        oldBtn->setChecked(false);
    }

    // Устанавливаем активную кнопку
    QPushButton *newBtn = m_buttons[index];
    newBtn->setProperty("active", true);
    newBtn->style()->unpolish(newBtn);
    newBtn->style()->polish(newBtn);
    newBtn->update();
    newBtn->setChecked(true);

    m_activeIndex = index;
    emit activeIndexChanged(index);
}

// ============================
// MainWindow
// ============================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    central = new QWidget(this);
    auto *vbox = new QVBoxLayout(central);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);

    // header
    // header = new HeaderWidget(this);
    // vbox->addWidget(header);

    // splitter
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(3);

    // Sidebar
    sidebar = new SidebarWidget(this);
    splitter->addWidget(sidebar);

    // Center widget
    QTabWidget* centerWidget = new QTabWidget();
    centerWidget->setObjectName("mainTabWidget");
    centerWidget->setAttribute(Qt::WA_StyledBackground, true);
    centerWidget->tabBar()->hide();

    splitter->addWidget(centerWidget);

    connect(sidebar, &SidebarWidget::activeIndexChanged,
           centerWidget, &QTabWidget::setCurrentIndex);



    //splitter->addWidget(preview);

    QSplitter* eventSplitter = new QSplitter(Qt::Horizontal);
    preview = new PreViewWidget(eventSplitter);
    preview->onTableRowClicked(-1);
    eventSplitter->setHandleWidth(3);
    eventSplitter->addWidget(createEventWidget());
    eventSplitter->addWidget(preview);
    eventSplitter->setStretchFactor(0, 1);
    eventSplitter->setStretchFactor(1, 1);

    QSplitter* quizSplitter = new QSplitter(Qt::Horizontal);
    quizPreview = new QuizPreViewWidget(quizSplitter);
    quizPreview->onTableRowClicked(-1);
    quizSplitter->setHandleWidth(3);
    quizSplitter->addWidget(createQuizWidget());
    quizSplitter->addWidget(quizPreview);
    quizSplitter->setStretchFactor(0, 1);
    quizSplitter->setStretchFactor(1, 1);

    centerWidget->addTab(eventSplitter, "");
    centerWidget->addTab(quizSplitter, "");
    centerWidget->addTab(createStatisticWidget(), "");

    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    vbox->addWidget(splitter);

    setCentralWidget(central);
    resize(1400, 900);

    QFile file(QApplication::applicationDirPath() + "/themes/styles2.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = file.readAll().trimmed();
        this->setStyleSheet(styleSheet);
        file.close();
    }
}

QWidget* MainWindow::createEventWidget()
{
    QWidget *w = new QWidget(this);
    w->setObjectName("eventWidget");
    auto *vbox = new QVBoxLayout(w);
    vbox->setContentsMargins(30, 30, 30, 30);
    vbox->setSpacing(5);

    QLabel* title = new QLabel("Мероприятия", w);
    title->setProperty("cssClass", "title");
    vbox->addWidget(title);

    QWidget* cont = new QWidget(this);
    cont->setProperty("cssClass", "whitebox");
    QHBoxLayout* contlay = new QHBoxLayout(cont);
    contlay->setContentsMargins(0, 0, 0, 0);

    QLineEdit* searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Поиск...");

    addEventButton = new QPushButton("Создать событие");
    addEventButton->setProperty("cssClass", "createButton");

    connect(addEventButton, &QPushButton::clicked, this, &MainWindow::onAddEventButtonClicked);

    contlay->addStretch();
    contlay->addWidget(searchEdit);
    contlay->addWidget(addEventButton);

    vbox->addWidget(cont);

    // QSqlTableModel *model = new QSqlTableModel();
    // model->setTable("event");  // имя вашей таблицы
    // model->setEditStrategy(QSqlTableModel::OnManualSubmit); // стратегия редактирования
    // model->select(); // загружает данные из базы


    // Table
    tableView = new QTableView(this);
    tableView->setProperty("cssClass", "dataTable");
    eventsModel = new EventsModel(this);
    eventsModel->loadSampleData();
    tableView->setModel(eventsModel);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setAlternatingRowColors(true);
    tableView->verticalHeader()->hide();
    tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive);
    tableView->setSortingEnabled(true);
    tableView->horizontalHeader()->setSortIndicatorShown(true);

    //connect(tableView, &QTableView::clicked, preview, &PreViewWidget::onTableRowClicked);


    // Прокси-модель
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(eventsModel);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(1); // предполагаем, что колонка 0 — "название"
    // Привязка proxy к таблице
    tableView->setModel(proxyModel);
    // Подключаем сигнал поиска
    connect(searchEdit, &QLineEdit::textChanged,
            proxyModel, &QSortFilterProxyModel::setFilterFixedString);


    connect(tableView, &QTableView::clicked, this, [&](){
        preview->onTableRowClicked(proxyModel->index(tableView->currentIndex().row(), 0).data().toInt());
    });

    vbox->addWidget(tableView);

    return w;
}

QWidget* MainWindow::createQuizWidget()
{
    QWidget *w = new QWidget(this);
    w->setObjectName("eventWidget");
    auto *vbox = new QVBoxLayout(w);
    vbox->setContentsMargins(30, 30, 30, 30);
    vbox->setSpacing(5);

    QLabel* title = new QLabel("Квизы", w);
    title->setProperty("cssClass", "title");
    vbox->addWidget(title);

    QWidget* cont = new QWidget(this);
    cont->setProperty("cssClass", "whitebox");
    QHBoxLayout* contlay = new QHBoxLayout(cont);
    contlay->setContentsMargins(0, 0, 0, 0);

    QLineEdit* searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Поиск...");

    addQuizButton = new QPushButton("Создать квиз");
    addQuizButton->setProperty("cssClass", "createButton");

    connect(addQuizButton, &QPushButton::clicked, this, &MainWindow::onAddQuizButtonClicked);

    contlay->addStretch();
    contlay->addWidget(searchEdit);
    contlay->addWidget(addQuizButton);

    vbox->addWidget(cont);

    // Table
    quizView = new QTableView(this);
    quizView->setProperty("cssClass", "dataTable");
    quizModel = new QuizModel(this);
    quizModel->loadSampleData();
    quizView->setModel(quizModel);
    quizView->horizontalHeader()->setStretchLastSection(true);
    quizView->setSelectionBehavior(QAbstractItemView::SelectRows);
    quizView->setSelectionMode(QAbstractItemView::SingleSelection);
    quizView->setAlternatingRowColors(true);
    quizView->verticalHeader()->hide();
    quizView->setSortingEnabled(true);
    quizView->horizontalHeader()->setSortIndicatorShown(true);

    quizProxyModel = new QSortFilterProxyModel(this);
    quizProxyModel->setSourceModel(quizModel);
    quizProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    quizProxyModel->setFilterKeyColumn(1); // предполагаем, что колонка 0 — "название"
    // Привязка proxy к таблице
    quizView->setModel(quizProxyModel);
    // Подключаем сигнал поиска
    connect(searchEdit, &QLineEdit::textChanged,
            quizProxyModel, &QSortFilterProxyModel::setFilterFixedString);


    connect(quizView, &QTableView::clicked, this, [&](){
        quizPreview->onTableRowClicked(quizProxyModel->index(quizView->currentIndex().row(), 0).data().toInt());
    });

    vbox->addWidget(quizView);

    return w;
}

QWidget* MainWindow::createStatisticWidget()
{
    QWidget *w = new QWidget(this);
    w->setObjectName("eventWidget");
    auto *vbox = new QVBoxLayout(w);
    vbox->setContentsMargins(30, 30, 30, 30);
    vbox->setSpacing(5);

    QLabel* title = new QLabel("Статистика", w);
    title->setProperty("cssClass", "title");
    vbox->addWidget(title);

    QPushButton* generateTeamReportButton = new QPushButton("Сформировать отчёт по командам");
    generateTeamReportButton->setProperty("cssClass", "createButton");
    QPushButton* generateUserReportButton = new QPushButton("Сформировать отчёт по участникам");
    generateUserReportButton->setProperty("cssClass", "createButton");
    QPushButton* generateEventReportButton = new QPushButton("Сформировать отчёт по мероприятию");
    generateEventReportButton->setProperty("cssClass", "createButton");

    generateTeamReportButton->setFixedWidth(300);
    generateUserReportButton->setFixedWidth(300);
    generateEventReportButton->setFixedWidth(300);

    QWidget* cont1 = new QWidget();
    cont1->setProperty("cssClass", "container");
    QHBoxLayout* lay1 = new QHBoxLayout(cont1);
    lay1->setContentsMargins(0, 0, 0, 0);
    lay1->setSpacing(0);
    QWidget* cont2 = new QWidget();
    cont2->setProperty("cssClass", "container");
    QHBoxLayout* lay2 = new QHBoxLayout(cont2);
    lay2->setContentsMargins(0, 0, 0, 0);
    lay2->setSpacing(0);

    cont1->setFixedWidth(250);
    cont2->setFixedWidth(250);

    QDateEdit* dateEdit1 = new QDateEdit(QDate::currentDate());
    dateEdit1->setCalendarPopup(true);
    dateEdit1->setDisplayFormat("dd.MM.yyyy");

    QDateEdit* dateEdit2 = new QDateEdit(QDate::currentDate());
    dateEdit2->setCalendarPopup(true);
    dateEdit2->setDisplayFormat("dd.MM.yyyy");

    QComboBox* hour1 = new QComboBox;
    QComboBox* hour2 = new QComboBox;
    for (int i = 0; i < 24; ++i) {
        hour1->addItem(QString("%1").arg(i, 2, 10, QLatin1Char('0')));
        hour2->addItem(QString("%1").arg(i, 2, 10, QLatin1Char('0')));
    }

    QComboBox* minute1 = new QComboBox;
    QComboBox* minute2 = new QComboBox;
    for (int i = 0; i < 60; i += 5) {
        minute1->addItem(QString("%1").arg(i, 2, 10, QLatin1Char('0')));
        minute2->addItem(QString("%1").arg(i, 2, 10, QLatin1Char('0')));
    }

    QLabel* s = new QLabel("с");
    s->setProperty("cssClass", "subtitle");
    QLabel* po = new QLabel("по");
    po->setProperty("cssClass", "subtitle");

    lay1->addWidget(s);
    lay1->addStretch();
    lay1->addWidget(dateEdit1);
    lay1->addWidget(hour1);
    lay1->addWidget(minute1);

    lay2->addWidget(po);
    lay2->addStretch();
    lay2->addWidget(dateEdit2);
    lay2->addWidget(hour2);
    lay2->addWidget(minute2);

    QComboBox* eventCombo = new QComboBox(this);
    eventCombo->setEditable(true);

    DatabaseManager& bd = DatabaseManager::instance();

    auto events = bd.listEvents();

    QStandardItemModel *model = new QStandardItemModel(this);
    for (const auto &q : events) {
        QStandardItem *item = new QStandardItem(q["title"].toString());
        item->setData(q["event_id"].toInt(), Qt::UserRole);        // userData сюда
        model->appendRow(item);
    }

    QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy->setFilterRole(Qt::DisplayRole); // фильтруем по названию
    proxy->setFilterKeyColumn(0);
    proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxy->sort(0, Qt::AscendingOrder);

    eventCombo->setModel(proxy);

    vbox->addWidget(cont1, 0, Qt::AlignLeft);
    vbox->addWidget(cont2, 0, Qt::AlignLeft);
    vbox->addWidget(eventCombo, 0, Qt::AlignLeft);
    vbox->addWidget(generateTeamReportButton, 0, Qt::AlignLeft);
    vbox->addWidget(generateUserReportButton, 0, Qt::AlignLeft);
    vbox->addWidget(generateEventReportButton, 0, Qt::AlignLeft);

    connect(generateTeamReportButton, &QPushButton::clicked, this, [dateEdit1, dateEdit2, hour1, hour2, minute1, minute2](){
        ReportHelper::reportTeams(QDateTime(dateEdit1->date(), QTime(hour1->currentText().toInt(), minute1->currentText().toInt())),
                                  QDateTime(dateEdit2->date(), QTime(hour2->currentText().toInt(), minute2->currentText().toInt())));
    });

    connect(generateUserReportButton, &QPushButton::clicked, this, [dateEdit1, dateEdit2, hour1, hour2, minute1, minute2](){
        ReportHelper::reportUsers(QDateTime(dateEdit1->date(), QTime(hour1->currentText().toInt(), minute1->currentText().toInt())),
                                  QDateTime(dateEdit2->date(), QTime(hour2->currentText().toInt(), minute2->currentText().toInt())));
    });

    connect(generateEventReportButton, &QPushButton::clicked, this, [dateEdit1, dateEdit2, hour1, hour2, minute1, minute2, eventCombo](){
        ReportHelper::reportQuiz(eventCombo->currentData(Qt::UserRole).toInt());
    });

    vbox->addStretch();

    return w;
}

// ============================
// Slots
// ============================
void MainWindow::onAddEventButtonClicked()
{
// 1. Загружаем список квизов из БД
    DatabaseManager& dbManager = DatabaseManager::instance();

    auto quizList = dbManager.listQuizzes();
    QList<QuizItem> quizzes;

    for (const auto& quiz : quizList) {
        quizzes.append(QuizItem{quiz["quiz_id"].toInt(), quiz["topic"].toString()});
    }

    // 2. Создаём диалог
    CreateEventDialog *dlg = new CreateEventDialog(quizzes, this);

    // 3. Обрабатываем сигнал eventCreated
    connect(dlg, &CreateEventDialog::eventCreated,
            this, [&](const Event &ev, int quizId)
    {
        eventsModel->addEvent(ev, quizId);

        eventsModel->loadSampleData();
    });

    // 4. Показываем диалог
    dlg->exec();
    dlg->deleteLater();

}

void MainWindow::onAddQuizButtonClicked()
{
    // 2. Создаём диалог
    CreateQuizDialog *dlg = new CreateQuizDialog(this);

    // 3. Обрабатываем сигнал eventCreated
    connect(dlg, &CreateQuizDialog::eventCreated,
            this, [&](const Quiz &ev)
            {
                quizModel->addQuiz(ev);
                quizModel->loadSampleData();
            });

    // 4. Показываем диалог
    dlg->exec();
    dlg->deleteLater();

}
