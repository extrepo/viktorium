#include "mainwindow.h"
#include "databasemanager.h"
#include "createeventdialog.h"
#include "participantsmodel.h"

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
        { QStringLiteral("Участники"),     QStringLiteral("assets/ic_participants.svg") },
        { QStringLiteral("Настройки"),     QStringLiteral("assets/ic_settings.svg") }
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

PreViewWidget::PreViewWidget(QWidget *parent): QWidget(parent)
{
    setObjectName("previewWidget");
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(5);
    setAttribute(Qt::WA_StyledBackground, true);

    QWidget* titleContainer = new QWidget(this);
    titleContainer->setProperty("cssClass", "container");
    QHBoxLayout* titleLayout = new QHBoxLayout(titleContainer);
    titleLayout->setContentsMargins(0, 0, 0, 0);

    title = new QLabel("Мероприятиe", this);
    title->setProperty("cssClass", "title");
    title->setAlignment(Qt::AlignCenter);
    type = new QLabel("Индивидуальный", this);
    type->setProperty("cssClass", "subtitle");
    type->setAlignment(Qt::AlignCenter);

    date = new QLabel();
    date->setProperty("cssClass", "date");
    time = new QLabel();
    time->setProperty("cssClass", "time");

    titleLayout->addWidget(title, 0, Qt::AlignRight | Qt::AlignVCenter);
    titleLayout->addWidget(type, 0, Qt::AlignRight | Qt::AlignVCenter);
    titleLayout->addStretch();

    QLabel* quizLabel = new QLabel("Квиз");
    quizLabel->setProperty("cssClass", "title2");

    quizComboBox = new QComboBox();
    quizComboBox->setEditable(false);
    quizComboBox->setEnabled(false);

    QLabel* participantsLabel = new QLabel("Участники");
    participantsLabel->setProperty("cssClass", "title2");

    participantSelectorWidget = new ParticipantSelectorWidget(this);

    // Добавить участников из базы
//    participantsWidget->addExistingParticipant({"Иван", "Иванов", "Иванович"});
//    participantsWidget->addExistingParticipant({"Петр", "Петров", ""});


    QLabel* settingsLabel = new QLabel("Настройки");
    settingsLabel->setProperty("cssClass", "title2");

    layout->addWidget(titleContainer);
    layout->addWidget(date);
    layout->addWidget(time);
    layout->addWidget(quizLabel);
    layout->addWidget(quizComboBox);
    layout->addWidget(participantsLabel);
    layout->addWidget(participantSelectorWidget);
    layout->addStretch();
    layout->addWidget(settingsLabel);
    layout->addStretch();
}

void PreViewWidget::onTableRowClicked(int index)
{
    DatabaseManager& db = DatabaseManager::instance();

    QVariantMap event = db.getEvent(index);

    QLocale ru(QLocale::Russian);

    title->setText(event["title"].toString());
    type->setText(event["type"].toInt() == 0 ? "Индивидуальный" : "Командный");
    QDateTime dateTime = QDateTime::fromSecsSinceEpoch(event["time"].toLongLong());
    date->setText(ru.toString(dateTime.date(), "d MMMM yyyy"));
    time->setText(dateTime.time().toString("HH:mm"));

    //QVariantMap quiz = db.getQuiz(event["quiz_id"].toInt());
    QVector<QVariantMap> quizes = db.listQuizzes();

    QStandardItemModel *model = new QStandardItemModel(this);
    for (const auto&q : quizes) {
        QStandardItem *item = new QStandardItem(q["topic"].toString());
        item->setData(q["quiz_id"].toInt(), Qt::UserRole);        // userData сюда
        model->appendRow(item);
    }

    QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy->setFilterRole(Qt::DisplayRole); // фильтруем по названию
    proxy->setFilterKeyColumn(0);
    proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxy->sort(0, Qt::AscendingOrder);

    quizComboBox->setModel(proxy);
    quizComboBox->setCurrentIndex(quizComboBox->findData(event["quiz_id"].toInt(), Qt::UserRole));

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
    header = new HeaderWidget(this);
    vbox->addWidget(header);

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

    preview = new PreViewWidget();

    splitter->addWidget(preview);

    centerWidget->addTab(createEventWidget(), "");
    centerWidget->addTab(createQuizWidget(), "");



    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setStretchFactor(2, 1);

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
    vbox->setContentsMargins(10, 10, 10, 10);
    vbox->setSpacing(5);

    QLabel* title = new QLabel("Мероприятия", w);
    title->setProperty("cssClass", "title");
    vbox->addWidget(title);

    QWidget* cont = new QWidget(this);
    cont->setProperty("cssClass", "whitebox");
    QHBoxLayout* contlay = new QHBoxLayout(cont);
    contlay->setContentsMargins(0, 0, 0, 0);

    QLabel* subtitle = new QLabel(w);
    QString text = "6 мероприятий · ближайшее сегодня";
    subtitle->setText(text);
    subtitle->setProperty("cssClass", "subtitle");

    QLineEdit* searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Поиск...");

    addEventButton = new QPushButton("Создать событие");
    addEventButton->setProperty("cssClass", "createButton");

    connect(addEventButton, &QPushButton::clicked, this, &MainWindow::onAddEventButtonClicked);

    contlay->addWidget(subtitle);
    contlay->addStretch();
    contlay->addWidget(searchEdit);
    contlay->addWidget(addEventButton);

    vbox->addWidget(cont);

    QSqlTableModel *model = new QSqlTableModel();
    model->setTable("event");  // имя вашей таблицы
    model->setEditStrategy(QSqlTableModel::OnManualSubmit); // стратегия редактирования
    model->select(); // загружает данные из базы


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
    tableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Interactive);
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
    auto *vbox = new QVBoxLayout(w);
    vbox->setContentsMargins(12, 12, 12, 12);
    vbox->setSpacing(8);

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
        // Добавляем событие в EventsModel
        eventsModel->addEvent(ev, quizId);

        eventsModel->loadSampleData();

        // Обновляем таблицу
        //tableView->reset();


    });

    // 4. Показываем диалог
    dlg->exec();
    dlg->deleteLater();

}
