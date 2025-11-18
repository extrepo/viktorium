#include "mainwindow.h"
#include "databasemanager.h"

#include <QHeaderView>
#include <QFile>
#include <QApplication>
#include <QDebug>
#include <QButtonGroup>


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
        { QStringLiteral("Отчёты"),        QStringLiteral("assets/ic_reports.svg") },
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
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(5);
    setAttribute(Qt::WA_StyledBackground, true);

    QWidget* titleContainer = new QWidget(this);
    titleContainer->setProperty("cssClass", "container");
    QHBoxLayout* titleLayout = new QHBoxLayout(titleContainer);
    //titleLayout->setContentsMargins(0, 0, 0, 0);

    title = new QLabel("Мероприятиe", this);
    title->setProperty("cssClass", "title");
    type = new QLabel("Индивидуальный", this);
    type->setProperty("cssClass", "subtitle");

    titleLayout->addWidget(title);
    titleLayout->addWidget(type);

    layout->addWidget(titleContainer);
    layout->addStretch();

}

void PreViewWidget::onTableRowClicked(const QModelIndex &index)
{
    int id = index.row();
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
    connect(tableView, &QTableView::clicked, preview, &PreViewWidget::onTableRowClicked);


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


    vbox->addWidget(tableView);

    // Bottom buttons
    QWidget *bottom = new QWidget(this);
    auto *hbox = new QHBoxLayout(bottom);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(12);

    btnOpen = new QPushButton("Открыть");
    btnEdit = new QPushButton("Редактировать");
    btnExport = new QPushButton("Экспорт");

    hbox->addWidget(btnOpen);
    hbox->addWidget(btnEdit);
    hbox->addWidget(btnExport);
    hbox->addStretch();

    vbox->addWidget(bottom);

    // Connect handlers
    connect(btnOpen, &QPushButton::clicked, this, &MainWindow::onOpenClicked);
    connect(btnEdit, &QPushButton::clicked, this, &MainWindow::onEditClicked);
    connect(btnExport, &QPushButton::clicked, this, &MainWindow::onExportClicked);

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
    // TODO
}

void MainWindow::onOpenClicked()
{
    // TODO
}

void MainWindow::onEditClicked()
{
    // TODO
}

void MainWindow::onExportClicked()
{
    // TODO
}
