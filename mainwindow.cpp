#define MAX_PRCSN 1000000000 /* 9 */
#define MIN_PRCSN 100 /* 2 */

#include "mainwindow.h"
#include "tablemodel.h"
#include "algorithms.h"
#include "delegate.h"
#include "graphic.h"
#include "emrmodel.h"

#include <QTime>
#include <QFile>
#include <QtMath>
#include <QLabel>
#include <QAction>
#include <QMenuBar>
#include <QToolBar>
#include <QSpinBox>
#include <QComboBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QTableView>
#include <QTabWidget>
#include <QStatusBar>
#include <QClipboard>
#include <QDockWidget>
#include <QGridLayout>
#include <QCloseEvent>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QTextStream>
#include <QInputDialog>
#include <QApplication>
#include <QProgressDialog>
#include <QTextDocumentWriter>

#include "3rdparty/qtxlsx/src/xlsx/xlsxdocument.h" // Библиотека для чтения xlsx-файлов https://github.com/dbzhang800/QtXlsxWriter
//#include <tbb/parallel_for.h>
//#include <tbb/task_group.h>

#include <QtConcurrent/QtConcurrentRun>
#include <QFutureSynchronizer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    createMenus();
    createBars();
    createDockWidget();

    clipboard = QApplication::clipboard();
    temp = new QFile("temp", this);

    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);
    tabWidget->setStatusTip(tr("Таблица расчётов"));
    tabWidget->setTabsClosable(true);
    tabWidget->addTab(createTable(), tr("Новая таблица"));

    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::on_tabWidget_currentChanged);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::on_tabClose_requested);
}

MainWindow::~MainWindow()
{
    delete tabWidget;
}

void
MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("Файл"));
    openTableAct = fileMenu->addAction(QIcon(":/images/open.png"), tr("&Открыть..."), this,
                                       &MainWindow::open, QKeySequence::Open);
    fileMenu->addSeparator();
    save_menu = new QMenu(tr("Сохранить как..."), fileMenu);
    save_menu->setIcon(QIcon(":/images/saveas.png"));
    saveTableAct = save_menu->addAction(QIcon(":/images/csv.png"), tr(".csv"), this,
                                        &MainWindow::writeToFile, QKeySequence::Save);
    saveAsDocumentAct = save_menu->addAction(QIcon(":/images/odf.png"), tr(".odf"), this,
                                             &MainWindow::writeAsDocument);
    save_as_xlsx_action = save_menu->addAction(QIcon(":/images/excel.png"), tr(".xlsx"), this,
                                               &MainWindow::save_as_xlsx, QKeySequence::SaveAs);
    fileMenu->addMenu(save_menu);
    fileMenu->addSeparator();
    quitAppAct = fileMenu->addAction(QIcon(":/images/quit.png"), tr("Выход"), this,
                                     &MainWindow::close, QKeySequence::Quit);
    openTableAct->setStatusTip(tr("Открыть файл таблицы *.csv"));
    saveTableAct->setStatusTip(tr("Сохранить таблицу в файл *.csv"));
    saveAsDocumentAct->setStatusTip(tr("Сохранить таблицу как документ *.odf"));
    quitAppAct->setStatusTip(tr("Выход из программы"));

    editMenu = menuBar()->addMenu(tr("Сервис"));
    showConvertWidgetAct = new QAction(QIcon(":/images/calc.png"), tr("Панель преобразований"), editMenu);
    showConvertWidgetAct->setCheckable(true);
    showConvertWidgetAct->setStatusTip(tr("Показать панель преобразования величин"));
    editMenu->addAction(showConvertWidgetAct);
    editMenu->addSeparator();
    addTabAct = editMenu->addAction(QIcon(":/images/add.png"), tr("Добавить вкладку"), this,
                                    &MainWindow::on_addTabAct_triggered, QKeySequence::AddTab);
    editTabAct = editMenu->addAction(QIcon(":/images/edit.png"), tr("Изменить название вкладки"), this,
                                     &MainWindow::on_editTabAct_triggered);
    editMenu->addSeparator();
    clearAct = editMenu->addAction(QIcon(":/images/clear.png"), tr("Очистить таблицу"), this,
                                   &MainWindow::on_clearAct_triggered);
    clearAct->setStatusTip(tr("Очистить содержимое текущей таблицы"));

    aboutMenu = menuBar()->addMenu(tr("Справка"));
    helpAct = aboutMenu->addAction(QIcon(":/images/question.png"), tr("Помощь"), this, &MainWindow::help, QKeySequence::HelpContents);
    aboutMenu->addSeparator();
    aboutAct = aboutMenu->addAction(tr("О программе"), this, &MainWindow::about);
    aboutQtAct = aboutMenu->addAction(tr("О Qt"), this, &QApplication::aboutQt);
    aboutAct->setStatusTip(tr("Информация"));
    helpAct->setStatusTip(tr("Справка по программе"));
    aboutQtAct->setStatusTip(tr("О фреймворке Qt"));

    connect(showConvertWidgetAct, &QAction::toggled, [&] (bool state) { state ? calcToolBar->show()
                                                                              : calcToolBar->hide(); } );
}

void
MainWindow::createDockWidget()
{
    dock = new QDockWidget(this);
    emrview = new QTableView(dock);
    emrmodel = new EmrModel(emrview);
    Delegate *emrdelegate = new Delegate(emrview);

    dock->setWidget(emrview);
    emrview->setModel(emrmodel);
    emrview->setItemDelegate(emrdelegate);
    emrview->setSelectionBehavior(QAbstractItemView::SelectItems);
    emrview->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    emrview->setContextMenuPolicy(Qt::CustomContextMenu);

    addDockWidget(Qt::BottomDockWidgetArea, dock);
    dock->hide();

    connect(emrview->itemDelegate(), &Delegate::closeEditor, this, &MainWindow::on_emrCloseEditor_emitted);
    connect(emrmodel, &QAbstractTableModel::dataChanged, this, &MainWindow::emrChangeData);
    connect(emrview, &QWidget::customContextMenuRequested, this, &MainWindow::emrCustomMenuRequested);
}

void
MainWindow::createBars()
{
    statusBar()->show();

    leftToolBar = new QToolBar(tr("Таблица"), this);
    leftToolBar->addAction(addTabAct);
    leftToolBar->addAction(editTabAct);
    leftToolBar->addSeparator();
    returnAct = leftToolBar->addAction(QIcon(":/images/return.png"), tr("Вернуть таблицу"),
                                       this, &MainWindow::openTempFile);
    leftToolBar->addSeparator();
    decimaMoreAct = leftToolBar->addAction(QIcon(":/images/decimal_more.png"), tr("Увеличить разрядность"), this,
                                           &MainWindow::incrementPrecision);
    decimaLessAct = leftToolBar->addAction(QIcon(":/images/decimal_less.png"), tr("Уменьшить разрядность"), this,
                                           &MainWindow::decrementPrecision);
    leftToolBar->addSeparator();
    showGraphicAct = leftToolBar->addAction(QIcon(":/images/graphic.png"), tr("Показать график"), this,
                                            &MainWindow::on_showGraphicAct_triggered);
    leftToolBar->setMovable(false);
    addTabAct->setStatusTip(tr("Добавить новую вкладку с пустой таблицей"));
    editTabAct->setStatusTip(tr("Изменить название текущей вкладки"));
    decimaMoreAct->setStatusTip(tr("По умолчанию: 5"));
    decimaLessAct->setStatusTip(tr("По умолчанию: 5"));
    showGraphicAct->setToolTip(tr("Показать график"));
    showGraphicAct->setStatusTip(tr("Показать график рассеивания измерений относительно математического ожидания"));
    showGraphicAct->setEnabled(false);
    MainWindow::addToolBar(Qt::LeftToolBarArea, leftToolBar);

    controlToolBar = new QToolBar(tr("Измерения"), this);
    QLabel *presetLabel = new QLabel(controlToolBar);
    measureCountSpinBox = new QSpinBox(controlToolBar);
    presetAdd = new QLineEdit(controlToolBar);
    presetLabel->setText(tr("Уст. значение: "));
    presetLabel->setAlignment(Qt::AlignCenter);
    presetAdd->setStyleSheet("QLineEdit { margin: 3px }");
    presetAdd->setMinimumHeight(35);
    presetAdd->setMaximumWidth(100);
    presetAdd->setStatusTip(tr("Добавить установленное значение"));
    measureCountSpinBox->setRange(1, 30);
    measureCountSpinBox->setMinimumHeight(35);
    measureCountSpinBox->setMaximumWidth(100);
    measureCountSpinBox->setKeyboardTracking(false);
    measureCountSpinBox->setAccelerated(true);
    measureCountSpinBox->setReadOnly(true);
    measureCountSpinBox->setMouseTracking(false);
    measureCountSpinBox->setAlignment(Qt::AlignCenter);
    measureCountSpinBox->setStatusTip(tr("Задать количество измерений"));
    measureCountSpinBox->setToolTip(tr("Измерения"));
    measureCountSpinBox->setStyleSheet("QSpinBox { margin: 3px }");
    controlToolBar->addWidget(presetLabel);
    controlToolBar->addWidget(presetAdd);
    controlToolBar->addSeparator();
    controlToolBar->addWidget(measureCountSpinBox);
    controlToolBar->addSeparator();
    controlToolBar->addAction(showConvertWidgetAct);
    controlToolBar->addSeparator();
    showEmrTable = controlToolBar->addAction(QIcon(":/images/table.png"), tr("Аддитивная погрешность"));
    showEmrTable->setCheckable(true);
    showEmrTable->setStatusTip(tr("Показать таблицу расчёта аддитивной погрешности"));
    showEmrTable->setToolTip(tr("Аддитивная погрешность"));
    MainWindow::addToolBar(Qt::TopToolBarArea, controlToolBar);

    calcToolBar = new QToolBar(tr("Преобразования"), this);
    convertWidget = new QWidget(calcToolBar);
    QGridLayout *convertGrid = new QGridLayout(convertWidget);
    QLabel *convertLabel = new QLabel(tr("dB и %"), convertWidget);
    convertBox = new QComboBox(convertWidget);
    convertInput = new QLineEdit(convertWidget);
    convertResult = new QTextEdit(convertWidget);
    calcToolBar->setMovable(false);
    convertLabel->setAlignment(Qt::AlignCenter);
    convertResult->setReadOnly(true);
    convertResult->setAlignment(Qt::AlignCenter);
    convertResult->setMaximumWidth(120);
    convertResult->setMaximumHeight(50);
    convertResult->setFontPointSize(10);
    convertInput->setMaximumWidth(60);
    convertBox->addItem("dB");
    convertBox->addItem("%");
    convertBox->setMaximumWidth(45);
    convertBox->setEditable(false);
    convertGrid->addWidget(convertLabel, 0, 0, 1, 2);
    convertGrid->addWidget(convertInput, 1, 0);
    convertGrid->addWidget(convertBox, 1, 1);
    convertGrid->addWidget(convertResult, 2, 0, 1, 2);
    convertGrid->setMargin(2);
    convertGrid->setSpacing(4);
    convertWidget->setLayout(convertGrid);
    convertWidget->setMaximumHeight(100);
    convertWidget->setMaximumWidth(150);

    ratioWidget = new QWidget(calcToolBar);
    QGridLayout *ratioGrid = new QGridLayout(ratioWidget);
    QLabel *ratioLabel = new QLabel(tr("Соотношение"), ratioWidget);
    ratioBox = new QComboBox(ratioWidget);
    firstRatioInput = new QLineEdit(ratioWidget);
    secondRatioInput = new QLineEdit(ratioWidget);
    ratioResult = new QLineEdit(ratioWidget);
    ratioLabel->setAlignment(Qt::AlignCenter);
    ratioResult->setReadOnly(true);
    ratioResult->setAlignment(Qt::AlignCenter);
    ratioResult->setMaximumWidth(120);
    ratioResult->setMaximumHeight(45);
    firstRatioInput->setMaximumWidth(60);
    secondRatioInput->setMaximumWidth(60);
    ratioBox->addItem("P");
    ratioBox->addItem("U");
    ratioBox->setMaximumWidth(45);
    ratioBox->setEditable(false);
    ratioGrid->addWidget(ratioLabel, 0, 0, 1, 2);
    ratioGrid->addWidget(firstRatioInput, 1, 0, 1, 1);
    ratioGrid->addWidget(ratioBox, 1, 1, 1, 1);
    ratioGrid->addWidget(secondRatioInput, 2, 0, 1, 1);
    ratioGrid->addWidget(ratioResult, 3, 0, 1, 2);
    ratioGrid->setMargin(2);
    ratioGrid->setSpacing(4);

    ratioWidget->setLayout(ratioGrid);
    ratioWidget->setMaximumHeight(150);
    ratioWidget->setMaximumWidth(150);
    calcToolBar->addWidget(convertWidget);
    calcToolBar->addSeparator();
    calcToolBar->addWidget(ratioWidget);
    calcToolBar->addSeparator();

    calcToolBar->hide();
    MainWindow::addToolBar(Qt::RightToolBarArea, calcToolBar);

    connect(convertInput, &QLineEdit::returnPressed,
            [=]() { convertValues(convertBox->currentIndex()); });
    connect(convertBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::convertValues);
    connect(firstRatioInput, &QLineEdit::returnPressed, secondRatioInput,
            [=]() { secondRatioInput->setFocus(); secondRatioInput->selectAll(); });
    connect(secondRatioInput, &QLineEdit::returnPressed,
            [=]() { ratioValues(ratioBox->currentIndex()); });
    connect(ratioBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::ratioValues);
    connect(measureCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::on_measureCountSpinBox_valueChanged);
    connect(presetAdd, &QLineEdit::returnPressed, this, &MainWindow::on_presetAdd_returnPressed);
    connect(showEmrTable, &QAction::toggled, [&] (bool state) { state ? dock->show()
                                                                      : dock->hide(); } );
}

QTableView
*MainWindow::createTable()
{
    QTableView *table = new QTableView(tabWidget);
    TableModel *model = new TableModel(table);
    Delegate *delegate = new Delegate(table);
    QHeaderView *vh = new QHeaderView(Qt::Vertical, table);

    table->setModel(model);
    table->setItemDelegate(delegate);
    table->setContextMenuPolicy(Qt::CustomContextMenu);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setMinimumSectionSize(100);
    table->horizontalHeader()->setDefaultSectionSize(150);
    table->setSelectionBehavior(QAbstractItemView::SelectItems);
    table->setEditTriggers(QAbstractItemView::AnyKeyPressed | QAbstractItemView::DoubleClicked);
    table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table->setVerticalHeader(vh);
    table->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);

    connect(table->itemDelegate(), &Delegate::closeEditor, this, &MainWindow::on_closeEditor_emitted);
    connect(table, &QWidget::customContextMenuRequested, this, &MainWindow::customMenuRequested);
    connect(model, &QAbstractTableModel::dataChanged, this, &MainWindow::changeData);

    connect(table->selectionModel(), &QItemSelectionModel::currentChanged, this, [=](const QModelIndex i)
    { if (i.column() == model->PRES) { showGraphicAct->setEnabled(true); measureCountSpinBox->setReadOnly(false); }
      else { showGraphicAct->setEnabled(false); measureCountSpinBox->setReadOnly(true); }
      measureCountSpinBox->setValue(table->rowSpan(i.row(), model->PRES)); });

    return table;
}

void
MainWindow::convertValues(int index)
{
    double U(0.0), P(0.0);
    QString result;
    double var = convertInput->text().replace(QChar(','), QChar('.')).toDouble();
    if (!convertInput->text().isEmpty())
    {
        switch (index)
        {
            case 0:
                U = qPow(10, var / 20) * 100 - 100;
                P = qPow(10, var / 10) * 100 - 100;
                result = QString("U=%1%\nP=%2%").arg(U).arg(P);
                convertResult->setText(result);
            break;
            case 1:
                U = 20 * log10(1 + (var / 100));
                P = 10 * log10(1 + (var / 100));
                result = QString("U=%1dB\nP=%2dB").arg(U).arg(P);
                convertResult->setText(result);
            break;
            default:
                convertResult->setText(tr("Ошибка"));
        }
    }
}

void
MainWindow::ratioValues(int index)
{
    double U(0.0), P(0.0);
    double first = firstRatioInput->text().replace(QChar(','), QChar('.')).toDouble();
    double second = secondRatioInput->text().replace(QChar(','), QChar('.')).toDouble();
    QString result;
    if (!firstRatioInput->text().isEmpty() && !secondRatioInput->text().isEmpty())
    {
        switch (index)
        {
            case 0:
                P = 10 * log10(first / second);
                result = QString("P=%1dB").arg(P);
                ratioResult->setText(result);
            break;
            case 1:
                U = 20 * log10(first / second);
                result = QString("U=%1dB").arg(U);
                ratioResult->setText(result);
            break;
            default:
                ratioResult->setText(tr("Неверные данные"));
        }
    }
}

void
MainWindow::writeAsDocument()
{
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);

    QString text;
    QTextEdit doc;
    QTableView *table = dynamic_cast<QTableView*>(tabWidget->currentWidget());
    QTextDocumentWriter writer(QFileDialog::getSaveFileName(this, tr("Сохранить таблицу"),
                               tabWidget->tabText(tabWidget->currentIndex()).append(".odf"), "*.odf"));

    text = tr("<table align=\"center\"><caption><b>Обработка результатов измерений</b></caption><tr>");

    for(int column(0); column < table->horizontalHeader()->count(); ++column)
        text.append(tr("<td align=\"center\">%1</td>")
                    .arg(table->model()->headerData(column, Qt::Horizontal).toString()));
    text.append("</tr>");

    for(int row(0); row < table->verticalHeader()->count(); ++row)
    {
        text.append("<tr>");
        for(int column(0); column < table->horizontalHeader()->count(); ++column)
            text.append(QString("<td align=\"center\">%1</td>")
                        .arg(table->model()->data(table->model()->index(row, column), Qt::EditRole)
                        .toString().replace(QChar('.'), QChar(','))));
        text.append("</tr>");
     }

    doc.setHtml(text);
    writer.setFormat("odf");
    writer.write(doc.document());

    QGuiApplication::restoreOverrideCursor();
}

void
MainWindow::changeData(const QModelIndex &index)
{
    QTableView *table = dynamic_cast<QTableView*>(tabWidget->currentWidget());
    TableModel *model = dynamic_cast<TableModel*>(table->model());

    if (index.column() != model->MEAS)
        return;

    Calculations c(table, index);
    int topRow = getTopRow(table, index.row());

    model->setData(model->index(index.row(), model->ABS, QModelIndex()), roundV(c.abslt, prcsn));
    model->setData(model->index(index.row(), model->ERR, QModelIndex()), roundV(c.err, prcsn));

    QFutureSynchronizer<void> sync;
    for (int i(0); i < table->rowSpan(topRow, model->PRES); ++i)
        sync.addFuture(QtConcurrent::run([this, model, topRow, i, c]()
                     { model->setData(model->index(topRow + i, model->AVRG, QModelIndex()), roundV(c.avrg, prcsn));
                       model->setData(model->index(topRow + i, model->DEV, QModelIndex()), roundV(c.dev, prcsn));
                       model->setData(model->index(topRow + i, model->DISP, QModelIndex()), roundV(c.disp, prcsn));
                       model->setData(model->index(topRow + i, model->GRUBBS, QModelIndex()), roundV(c.grubbs.at(i), prcsn)); }));
    sync.waitForFinished();

    /*
    tbb::parallel_for(0, table->rowSpan(topRow, model->PRES), 1,
    [=](int i) { model->setData(model->index(topRow + i, model->AVRG, QModelIndex()), roundV(c.avrg, prcsn));
                 model->setData(model->index(topRow + i, model->DEV, QModelIndex()), roundV(c.dev, prcsn));
                 model->setData(model->index(topRow + i, model->DISP, QModelIndex()), roundV(c.disp, prcsn));
                 model->setData(model->index(topRow + i, model->GRUBBS, QModelIndex()), roundV(c.grubbs.at(i), prcsn)); });
                 */
}

void
MainWindow::on_closeEditor_emitted(QWidget *editor)
{
    QTableView *table = dynamic_cast<QTableView*>(tabWidget->currentWidget());
    TableModel *model = dynamic_cast<TableModel*>(table->model());
    QModelIndex i = table->indexAt(editor->pos());

    if (i.column() != model->PRES)
        return;

    int span = table->rowSpan(i.row(), model->PRES);
    int topRow = getTopRow(table, i.row());
    double meas(0), pres = model->data(i, Qt::EditRole).toDouble();


    for (int i(topRow); i < (topRow + span); ++i)
         model->setData(model->index(i, model->PRES), pres);

    double abslt(0.0), err(0.0);
    for(int row(0); row < span; ++row)
    {
        meas = model->data(model->index(i.row() + row, model->MEAS, QModelIndex()), Qt::EditRole).toDouble();

        abslt = meas - pres;
        err = abslt / pres * 100.0;
        model->setData(model->index(i.row() + row, model->ABS, QModelIndex()),
                       roundV(abslt, prcsn), Qt::EditRole);
        model->setData(model->index(i.row() + row, model->ERR, QModelIndex()),
                       roundV(err, prcsn), Qt::EditRole);
    }
}

void
MainWindow::incrementPrecision()
{
    (prcsn >= MAX_PRCSN) ? prcsn : prcsn *= 10;

    int count(0.0);
    for (int tmp(prcsn); tmp > 1; ++count, tmp /= 10);

    statusBar()->showMessage(tr("Разрядность: %1").arg(count), 5000);

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    TableModel *model = dynamic_cast<TableModel*>(dynamic_cast<QTableView*>(tabWidget->currentWidget())->model());

    QFutureSynchronizer<void> sync;
    for(int i(0); i < model->rowCount(); ++i)
    {
        sync.addFuture(QtConcurrent::run([this, model, i]() { changeData(model->index(i, model->MEAS)); }));
        sync.addFuture(QtConcurrent::run([this, i]() { emrChangeData(emrmodel->index(i, EmrColumns::MEASURE)); }));
    }
    sync.waitForFinished();

    /*
    tbb::parallel_for(0, model->rowCount(), 1, [this, model](int i) { changeData(model->index(i, model->MEAS)); });
    tbb::parallel_for(0, emrmodel->rowCount(), 1, [this](int i) {  emrChangeData(emrmodel->index(i, EmrColumns::MEASURE)); });
    */

/*
    tbb::task_group g_model, emr_model;
    for(int i(0); i < model->rowCount(); ++i)
        g_model.run([this, model, i]() { changeData(model->index(i, model->MEAS)); });

    if (emrmodel->rowCount() > 0)
    {
        for (int i(0); i < emrmodel->rowCount(); ++i)
             emr_model.run([this, i]() { emrChangeData(emrmodel->index(i, EmrColumns::MEASURE)); });
    }

    g_model.wait();
    emr_model.wait();
*/
    QGuiApplication::restoreOverrideCursor();

    model->refresh();
}

void
MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Загрузить таблицу"), "", "*.csv");
    if (fileName.isEmpty())
        return;
    else
        readFromFile(fileName);
}

void
MainWindow::decrementPrecision()
{
    (prcsn <= MIN_PRCSN) ? prcsn : prcsn /= 10;

    int count(0.0);
    for (int tmp(prcsn); tmp > 1; ++count, tmp /= 10);

    statusBar()->showMessage(tr("Разрядность: %1").arg(count), 5000);

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    TableModel *model = dynamic_cast<TableModel*>(dynamic_cast<QTableView*>(tabWidget->currentWidget())->model());

    QFutureSynchronizer<void> sync;
    for(int i(0); i < model->rowCount(); ++i)
    {
        sync.addFuture(QtConcurrent::run([this, model, i]() { changeData(model->index(i, model->MEAS)); }));
        sync.addFuture(QtConcurrent::run([this, i]() { emrChangeData(emrmodel->index(i, EmrColumns::MEASURE)); }));
    }
    sync.waitForFinished();

    /*
    tbb::parallel_for(0, model->rowCount(), 1, [this, model](int i) { changeData(model->index(i, model->MEAS)); });
    tbb::parallel_for(0, emrmodel->rowCount(), 1, [this](int i) {  emrChangeData(emrmodel->index(i, EmrColumns::MEASURE)); });
    */

    /*
    tbb::task_group g_model, emr_model;
    for(int i(0); i < model->rowCount(); ++i)
        g_model.run([this, model, i]() { changeData(model->index(i, model->MEAS)); });

    if (emrmodel->rowCount() > 0)
    {
        for (int i(0); i < emrmodel->rowCount(); ++i)
             emr_model.run([this, i]() { emrChangeData(emrmodel->index(i, EmrColumns::MEASURE)); });
    }

    g_model.wait();
    emr_model.wait();
    */

    QGuiApplication::restoreOverrideCursor();

    model->refresh();
}

void
MainWindow::on_measureCountSpinBox_valueChanged(const int count)
{
    QTableView *table = dynamic_cast<QTableView*>(tabWidget->currentWidget());
    TableModel *model = dynamic_cast<TableModel*>(table->model());

    QModelIndex index =  table->selectionModel()->currentIndex();
    if (index.column() != model->PRES)
        return;

    int span = table->rowSpan(index.row(), model->PRES);
    if (count > span)
    {
        model->insertRows(index.row() + span, count - span);

        table->setSpan(index.row(), model->PRES, count, 1);
        table->setSpan(index.row(), model->AVRG, count, 1);
        table->setSpan(index.row(), model->DEV, count, 1);
        table->setSpan(index.row(), model->DISP, count, 1);

        for (int i(0); i < table->rowSpan(index.row(), model->PRES); ++i)
        {
            model->setData(model->index(index.row() + i, model->PRES),
                           model->data(model->index(index.row(), model->PRES), Qt::EditRole), Qt::EditRole);
            model->setMeasuresCount(index.row() + i, count);
            model->setLimit(index.row() + i, model->getLimit(index.row()));
        }

        QFutureSynchronizer<void> sync;
        for(int i(0); i < model->rowCount(); ++i)
            sync.addFuture(QtConcurrent::run([this, model, i]() { changeData(model->index(i, model->MEAS)); }));
        sync.waitForFinished();

        //tbb::parallel_for(0, model->rowCount(), 1, [=](int i) { changeData(model->index(i, model->MEAS)); });

        table->selectionModel()->clearSelection();
    }
    else if (count < span)
    {
        model->removeRows(index.row() + span - 1, span - count);
        if (count != 1)
        {
            table->setSpan(index.row(), model->PRES, count, 1);
            table->setSpan(index.row(), model->AVRG, count, 1);
            table->setSpan(index.row(), model->DEV, count, 1);
            table->setSpan(index.row(), model->DISP, count, 1);
        }

        for (int i(0); i < table->rowSpan(index.row(), model->PRES); ++i)
             model->setMeasuresCount(index.row() + i, count);

        QFutureSynchronizer<void> sync;
        for(int i(0); i < model->rowCount(); ++i)
            sync.addFuture(QtConcurrent::run([this, model, i]() { changeData(model->index(i, model->MEAS)); }));
        sync.waitForFinished();

        //tbb::parallel_for(0, model->rowCount(), 1, [=](int i) { changeData(model->index(i, model->MEAS)); });
    }
}

void
MainWindow::on_presetAdd_returnPressed()
{
    if (presetAdd->text().isEmpty())
        return;

    QTableView *table = dynamic_cast<QTableView*>(tabWidget->currentWidget());
    TableModel *model = dynamic_cast<TableModel*>(table->model());
    QVariant newValue = presetAdd->text().replace(QChar(','), QChar('.'));
    QModelIndex index;

    model->insertRow(model->rowCount());
    index =  model->index(model->rowCount() - 1, model->PRES);
    model->setData(index, newValue);
    table->scrollToBottom();

    presetAdd->clear();
}

bool
MainWindow::on_quitAppAct_triggered()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Выход из программы"), tr("Вы уверены?"),
                                        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
        return true;
    return false;
}

void
MainWindow::on_setLimitAct_triggered()
{
    QTableView *table = dynamic_cast<QTableView*>(tabWidget->currentWidget());
    TableModel *model = dynamic_cast<TableModel*>(table->model());
    if (!table->selectionModel()->hasSelection())
        return;

    QModelIndexList list = table->selectionModel()->selectedIndexes();

    bool isOk;
    QString newValue = QInputDialog::getText(this, tr("Допустимая погрешность"),
                                             tr("Введите значение:"), QLineEdit::Normal,
                                             QString(), &isOk);

    if (isOk && !newValue.isEmpty())
    {
        for (int i(0); i < list.size(); ++i)
             model->setLimit(list.at(i).row(), newValue.replace(QChar(','), QChar('.')).toDouble());
    }
}

void
MainWindow::on_setSpanAct_triggered()
{
    QTableView *table = dynamic_cast<QTableView*>(tabWidget->currentWidget());
    TableModel *model = dynamic_cast<TableModel*>(table->model());
    if (!table->selectionModel()->hasSelection())
        return;

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QModelIndexList list = table->selectionModel()->selectedIndexes();

    int span = list.last().row() - list.first().row() + 1;
    if (span != 1)
    {
        table->setSpan(list.first().row(), model->PRES, span, 1);
        table->setSpan(list.first().row(), model->AVRG, span, 1);
        table->setSpan(list.first().row(), model->DEV, span, 1);
        table->setSpan(list.first().row(), model->DISP, span, 1);

        for (int row(list.first().row()); row <= list.last().row(); ++row)
        {
            model->setData(model->index(row, model->PRES),
                           model->data(model->index(list.first().row(), model->PRES), Qt::EditRole));
            model->setData(model->index(row, model->AVRG),
                           model->data(model->index(list.first().row(), model->AVRG), Qt::EditRole));
            model->setData(model->index(row, model->DEV),
                           model->data(model->index(list.first().row(), model->DEV), Qt::EditRole));
            model->setData(model->index(row, model->DISP),
                           model->data(model->index(list.first().row(), model->DISP), Qt::EditRole));
        }

        for (int i(0); i < list.size(); ++i)
             model->setMeasuresCount(list.at(i).row(), span);

        QFutureSynchronizer<void> sync;
        for(int i(0); i < model->rowCount(); ++i)
            sync.addFuture(QtConcurrent::run([this, model, i]() { changeData(model->index(i, model->MEAS)); }));
        sync.waitForFinished();

        //tbb::parallel_for(0, model->rowCount(), 1, [=](int i) { changeData(model->index(i, model->MEAS)); });

    }
    QGuiApplication::restoreOverrideCursor();
}

void
MainWindow::on_showGraphicAct_triggered()
{
    Graphic g(tabWidget->currentWidget());
    g.exec();
}

void
MainWindow::on_resetSpanAct_triggered()
{
    QTableView *table = dynamic_cast<QTableView*>(tabWidget->currentWidget());
    TableModel *model = dynamic_cast<TableModel*>(table->model());

    if (!table->selectionModel()->hasSelection())
        return;

    QModelIndexList list = table->selectionModel()->selectedIndexes();

    if (table->rowSpan(list.first().row(), model->PRES) != 1)
    {
        table->setSpan(list.first().row(), model->PRES, 1, 1);
        table->setSpan(list.first().row(), model->AVRG, 1, 1);
        table->setSpan(list.first().row(), model->DEV, 1, 1);
        table->setSpan(list.first().row(), model->DISP, 1, 1);

        for (int i(0); i < list.size(); ++i)
             model->setMeasuresCount(list.at(i).row(), 1);
    }
}

void
MainWindow::on_addTabAct_triggered()
{
    bool isOk;
    QString newName = QInputDialog::getText(this, tr("Новая вкладка"),
                                            tr("Введите название:"), QLineEdit::Normal,
                                            QString(), &isOk);

    if (isOk && !newName.isEmpty())
        tabWidget->setCurrentIndex(tabWidget->addTab(createTable(), newName));
    else if (isOk && newName.isEmpty())
        QMessageBox::warning(this, tr("Ошибка"), tr("Пустое название"));
}

void
MainWindow::on_cleanItemsAct_triggered()
{
    QTableView *table = dynamic_cast<QTableView*>(tabWidget->currentWidget());
    TableModel *model = dynamic_cast<TableModel*>(table->model());

    QModelIndexList list = table->selectionModel()->selectedIndexes();

    for (auto it = list.begin(); it != list.end(); ++it)
         model->setData(*it, 0, Qt::EditRole);
}

void
MainWindow::on_clearAct_triggered()
{
    QTableView *table = dynamic_cast<QTableView*>(tabWidget->currentWidget());
    TableModel *model = dynamic_cast<TableModel*>(table->model());

    model->clear();
}

void
MainWindow::on_deleteRowAct_triggered()
{
    QTableView *table = dynamic_cast<QTableView*>(tabWidget->currentWidget());
    TableModel *model = dynamic_cast<TableModel*>(table->model());
    if (!table->selectionModel()->hasSelection())
        return;

    QModelIndexList list = table->selectionModel()->selectedIndexes();
    for (int i(list.last().row()); i >= list.first().row(); --i)
         model->removeRow(i);

    QFutureSynchronizer<void> sync;
    for(int i(0); i < model->rowCount(); ++i)
        sync.addFuture(QtConcurrent::run([this, model, i]() { changeData(model->index(i, model->MEAS)); }));
    sync.waitForFinished();

    //tbb::parallel_for(0, model->rowCount(), 1, [=](int i) { changeData(model->index(i, model->MEAS)); });
}

void
MainWindow::on_tabClose_requested(int index)
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Удаление вкладки"), tr("Вы уверены?"),
                                        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        if (tabWidget->count() == 1)
        {
            saveTempFile();
            tabHeaderName = tabWidget->tabText(0);

            QWidget *tableToDelete = tabWidget->widget(0);
            tabWidget->addTab(createTable(), "Новая таблица");
            tabWidget->removeTab(0);
            delete tableToDelete;
        } else
        {    
            saveTempFile();
            tabHeaderName = tabWidget->tabText(index);

            QWidget *tableToDelete = tabWidget->widget(index);
            tabWidget->removeTab(index);
            delete tableToDelete;
        }
    } else return;
}

void
MainWindow::on_editTabAct_triggered()
{
    bool isOk;
    QString newName = QInputDialog::getText(this, tr("Изменение вкладки"),
                                            tr("Введите новое название:"), QLineEdit::Normal,
                                            tabWidget->tabText(tabWidget->currentIndex()), &isOk);

    if (isOk && !newName.isEmpty())
        tabWidget->setTabText(tabWidget->currentIndex(), newName);
}

void
MainWindow::customMenuRequested(const QPoint pos)
{
    QTableView *table = dynamic_cast<QTableView*>(tabWidget->currentWidget());
    TableModel *model = dynamic_cast<TableModel*>(table->model());

    if (!table->selectionModel()->hasSelection())
        return;

    QModelIndex index = table->indexAt(pos);
    QModelIndexList list = table->selectionModel()->selectedIndexes();

    QMenu context_menu;
    QAction copy_action(QIcon(":/images/copy.png"), tr("Копировать"));
    QAction set_limit_action(QIcon(":/images/limit.png"), tr("Задать предел"));
    QAction delete_row_action(QIcon(":/images/deleterow.png"), tr("Удалить выбранные строки"));
    QAction set_span_action(QIcon(":/images/integration.png"), tr("Объединить"));
    QAction reset_span_action(tr("Разделить"));
    QAction clean_item_action(QIcon(":/images/clean.png"), tr("Очистить содержимое"));

    copy_action.setShortcut(QKeySequence::Copy);
    delete_row_action.setShortcut(QKeySequence::Delete);

    QMenu sub_menu(tr("Вставить строку"));
    QAction add_new_row_low_action(QIcon(":/images/down.png"), tr("Ниже"));
    QAction add_new_row_top_action(QIcon(":/images/up.png"), tr("Выше"));
    sub_menu.addAction(&add_new_row_low_action);
    sub_menu.addAction(&add_new_row_top_action);

    if (index.column() == model->PRES && list.size() > 1)
    {
        context_menu.addAction(&set_limit_action);
        context_menu.addSeparator();
        context_menu.addAction(&set_span_action);
        context_menu.addAction(&reset_span_action);
        context_menu.addSeparator();
    }
    if (index.column() == model->LIM && list.size() > 1)
    {
        context_menu.addAction(&set_limit_action);
    }

    context_menu.addAction(&copy_action);
    context_menu.addSeparator();
    context_menu.addMenu(&sub_menu);
    context_menu.addAction(&delete_row_action);
    context_menu.addSeparator();
    context_menu.addAction(&clean_item_action);

    QAction *act = context_menu.exec(table->viewport()->mapToGlobal(pos));
    if (!act)
        return;

    if (act == &copy_action)
        copy();
    else if (act == &delete_row_action)
        on_deleteRowAct_triggered();
    else if (act == &set_limit_action)
        on_setLimitAct_triggered();
    else if (act == &set_span_action)
        on_setSpanAct_triggered();
    else if (act == &reset_span_action)
        on_resetSpanAct_triggered();
    else if (act == &clean_item_action)
        on_cleanItemsAct_triggered();
    else if (act == &add_new_row_low_action)
        addRowTop(index);
    else if (act == &add_new_row_top_action)
        addRowTop(index);
    else return;
}

void
MainWindow::addRowLow(const QModelIndex &index)
{
    QTableView *table = dynamic_cast<QTableView*>(tabWidget->currentWidget());
    TableModel *model = dynamic_cast<TableModel*>(table->model());

    int topRow = getTopRow(table, index.row());
    int span = table->rowSpan(index.row(), model->PRES);

    model->insertRows(topRow + span, 1);
}

void
MainWindow::addRowTop(const QModelIndex &index)
{
    QTableView *table = dynamic_cast<QTableView*>(tabWidget->currentWidget());
    TableModel *model = dynamic_cast<TableModel*>(table->model());

    int topRow = getTopRow(table, index.row());
    model->insertRows(topRow, 1);
}

void
MainWindow::on_tabWidget_currentChanged(int index)
{
    QTableView *table = dynamic_cast<QTableView*>(tabWidget->widget(index));
    TableModel *model = dynamic_cast<TableModel*>(table->model());
    QModelIndex i = table->selectionModel()->currentIndex();

    if (table->selectionModel()->hasSelection() && i.column() == model->PRES)
    {
        measureCountSpinBox->setValue(table->rowSpan(i.row(), model->PRES));
        measureCountSpinBox->setReadOnly(false);
        showGraphicAct->setEnabled(true);
    }
    else
    {
        measureCountSpinBox->setValue(1);
        measureCountSpinBox->setReadOnly(true);
        showGraphicAct->setEnabled(false);
    }
}

void
MainWindow::closeEvent(QCloseEvent *event)
{
    on_quitAppAct_triggered() ? event->accept()
                              : event->ignore();
    temp->remove();
    temp->close();
}

void
MainWindow::readFromFile(QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::information(this, tr("Невозможно открыть файл"),
            file.errorString());
        return;
    }

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);

    QTextStream in(&file);
    QString str;
    QStringList strlist;

    int index = tabWidget->addTab(createTable(), fileName.section('/', -1));

    QTableView *table = dynamic_cast<QTableView*>(tabWidget->widget(index));
    TableModel *model = dynamic_cast<TableModel*>(table->model());

    while (!in.atEnd())
    {
        str = in.readLine();
        strlist << str.split(';');
    }

    file.close();

    QStringList::const_iterator it;
    for (it = strlist.constBegin(); it != strlist.constEnd();)
    {
        model->insertRow(model->rowCount());
        for (int columns(0); columns < model->columnCount(); ++columns, ++it)
        {
            if (it != strlist.constEnd())
            {
                str = *it;
                model->setData(model->index(model->rowCount() - 1, columns), str.replace(QChar(','), QChar('.')));
            } else
            {
                tabWidget->removeTab(index);
                delete table;
                QGuiApplication::restoreOverrideCursor();
                QMessageBox::critical(this, tr("Ошибка"), tr("Несовместимая таблица"));
                return;
            }
        }
    }

    QVariant current;
    QVariant previous = model->data(model->index(0, model->PRES), Qt::EditRole);
    for (int row(1), span(1); row < model->rowCount(); ++row)
    {
        current = model->data(model->index(row, model->PRES), Qt::EditRole);
        if (previous == current)
        {
            ++span;

            table->setSpan(row - span + 1, model->PRES, span, 1);
            table->setSpan(row - span + 1, model->AVRG, span, 1);
            table->setSpan(row - span + 1, model->DEV, span, 1);
            table->setSpan(row - span + 1, model->DISP, span, 1);

            for (int i(0); i < table->rowSpan(row - span + 1, model->PRES); ++i)
                 model->setMeasuresCount(row - span + i + 1, span);

            previous = current;
        }
        else
        {
            span = 1;
            previous = current;
        }
    }

    QFutureSynchronizer<void> sync;
    for(int i(0); i < model->rowCount(); ++i)
        sync.addFuture(QtConcurrent::run([this, model, i]() { changeData(model->index(i, model->MEAS)); }));
    sync.waitForFinished();

    //tbb::parallel_for(0, model->rowCount(), 1, [=](int i) { changeData(model->index(i, model->MEAS)); });

    model->removeRow(0);
    tabWidget->setCurrentIndex(index);

    QGuiApplication::restoreOverrideCursor();
}

void
MainWindow::writeToFile()
{
    QString standardName = tabWidget->tabText(tabWidget->currentIndex()).append(".csv");
    QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранить таблицу"), standardName, "*.csv");
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        QGuiApplication::setOverrideCursor(Qt::WaitCursor);

        QTableView *table = dynamic_cast<QTableView*>(tabWidget->currentWidget());
        QTextStream out(&file);
        QStringList strlist;

        for(int columns(0); columns < table->horizontalHeader()->count(); ++columns)
            strlist << table->model()->headerData(columns, Qt::Horizontal).toString().toUtf8();

        out << strlist.join(';')+'\n';

        for(int rows(0); rows < table->verticalHeader()->count(); ++rows)
        {
            strlist.clear();
            for(int columns(0); columns < table->horizontalHeader()->count(); ++columns)
                strlist << table->model()->data(table->model()->index(rows, columns), Qt::EditRole).toString();

            strlist.replaceInStrings(QChar('.'), QChar(','));
            out << strlist.join(';')+'\n';
        }

        file.close();

        QGuiApplication::restoreOverrideCursor();
    }
}

void MainWindow::save_as_xlsx()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Экспорт в xlsx"), QCoreApplication::applicationDirPath(), tr("Файл таблиц (*.xlsx)"));

    QXlsx::Document xlsx;
    QXlsx::Format format;
    format.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    format.setVerticalAlignment(QXlsx::Format::AlignVCenter);

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    for (int k = 0; k < tabWidget->count(); ++k)
    {
        xlsx.addSheet(tabWidget->tabText(k));
        QXlsx::Worksheet *sheet = xlsx.currentWorksheet();

        QTableView *table = dynamic_cast<QTableView*>(tabWidget->widget(k));
        TableModel *model = dynamic_cast<TableModel*>(table->model());

        /* Заполнение данными */
        for (int i = 0; i < model->rowCount(); ++i)
        {
            for (int j = 0; j < model->columnCount(); ++j)
            {
                QModelIndex index = model->index(i, j);
                sheet->writeString(1, j + 1, model->headerData(j, Qt::Horizontal, Qt::DisplayRole).toString());

                if (j == TableModel::LIM)
                    sheet->writeString(i + 2, j + 1, index.data(Qt::DisplayRole).toString());
                else
                    sheet->writeNumeric(i + 2, j + 1, index.data(Qt::DisplayRole).toDouble());

                QApplication::processEvents();
            }
        }

        /* Объединение ячеек */
        for (int row = 0; row < model->rowCount(); ++row)
        {
            int span = table->rowSpan(row, TableModel::PRES);
            if (span > 1)
            {
                sheet->mergeCells(QString("B%1:B%2").arg(row + 2).arg(row + span + 1), format);
                sheet->mergeCells(QString("F%1:F%2").arg(row + 2).arg(row + span + 1), format);
                sheet->mergeCells(QString("G%1:G%2").arg(row + 2).arg(row + span + 1), format);
                sheet->mergeCells(QString("H%1:H%2").arg(row + 2).arg(row + span + 1), format);
                row += (span - 1);
            }
        }
    }
    QGuiApplication::restoreOverrideCursor();

    if (fileName.section(QChar('.'), -1) != "xlsx")
        fileName.append(".xlsx");

    if (!xlsx.saveAs(fileName))
        statusBar()->showMessage(tr("%1 Ошибка при сохранении файла").arg(QTime::currentTime().toString()));
    else
        statusBar()->showMessage(tr("Файл успешно сохранён"), 5000);
}

void
MainWindow::saveTempFile()
{
    if (!temp->open(QIODevice::WriteOnly))
    {
        statusBar()->showMessage(tr("Устройство для записи недоступно"), 5000);
        return;
    }

    QTableView *table = dynamic_cast<QTableView*>(tabWidget->currentWidget());
    QTextStream out(temp);
    QStringList strlist;

    for(int columns(0); columns < table->horizontalHeader()->count(); ++columns)
        strlist << table->model()->headerData(columns, Qt::Horizontal).toString().toUtf8();
    out << strlist.join(';')+'\n';
    for(int rows(0); rows < table->verticalHeader()->count(); ++rows)
    {
        strlist.clear();
        for(int columns(0); columns < table->horizontalHeader()->count(); ++columns)
            strlist << table->model()->data(table->model()->index(rows, columns), Qt::EditRole).toString();
        out << strlist.join(';')+'\n';
     }
    temp->close();
}

void
MainWindow::openTempFile()
{
    if (!temp->open(QIODevice::ReadOnly))
    {
        statusBar()->showMessage(tr("Недавняя таблица недоступна"), 5000);
        return;
    }

    QString fileName = temp->fileName();
    readFromFile(fileName);
    tabWidget->setTabText(tabWidget->count() - 1, tabHeaderName);

    temp->close();
}

void
MainWindow::copy()
{
    QTableView *table = dynamic_cast<QTableView*>(tabWidget->currentWidget());

    if (!dynamic_cast<QTableView*>(tabWidget->currentWidget())->selectionModel()->hasSelection())
        return;

    QString copied, data;
    QModelIndexList list = table->selectionModel()->selectedIndexes();
    std::sort(list.begin(), list.end());

    for (int i(0); i < (list.size() - 1); ++i)
    {
        data = QString::number(list.at(i).data(Qt::EditRole).toDouble()).replace(QChar('.'), QChar(','));
        if (list.at(i).row() != list.at(i + 1).row())
            data.append('\n');
        else
            data.append('\t');

        copied.append(data);
    }

    copied.append(QString::number(list.last().data(Qt::EditRole).toDouble()).replace(QChar('.'), QChar(',')));
    clipboard->setText(copied);
}

void
MainWindow::emrChangeData(const QModelIndex &index)
{
    if (index.column() != EmrColumns::MEASURE)
        return;

    double range, res, unit, err, abs_err, limit, measure;

    range = emrmodel->data(emrmodel->index(index.row(), EmrColumns::RANGE), Qt::EditRole).toDouble();
    res = emrmodel->data(emrmodel->index(index.row(), EmrColumns::RESOLUTION), Qt::EditRole).toDouble();
    unit = emrmodel->data(emrmodel->index(index.row(), EmrColumns::UNIT), Qt::EditRole).toDouble();
    err = emrmodel->data(emrmodel->index(index.row(), EmrColumns::ERROR), Qt::EditRole).toDouble();
    measure = emrmodel->data(emrmodel->index(index.row(), EmrColumns::MEASURE), Qt::EditRole).toDouble();

    abs_err = (range / 100 * err) + (res * unit);
    limit = (measure / 100 * err) + (res * unit);

    emrmodel->setData(emrmodel->index(index.row(), EmrColumns::ABS_MIN), roundV(measure - limit, prcsn));
    emrmodel->setData(emrmodel->index(index.row(), EmrColumns::ABS_MAX), roundV(measure + limit, prcsn));
    emrmodel->setData(emrmodel->index(index.row(), EmrColumns::ABSOLUTE), roundV(abs_err, prcsn));
}

void
MainWindow::emrCustomMenuRequested(const QPoint pos)
{
    QModelIndexList list = emrview->selectionModel()->selectedIndexes();

    QMenu context_menu;
    QAction emr_copy_action(QIcon(":/images/copy.png"), tr("Копировать"));
    QAction emr_add_new_row_action(QIcon(":/images/add.png"), tr("Вставить строку"));
    QAction emr_delete_row_action(QIcon(":/images/delete.png"), tr("Удалить строки"));
    QAction emr_input_value_action(QIcon(":/images/input.png"), tr("Ввести значение"));

    if (list.size() > 1)
    {
        context_menu.addAction(&emr_input_value_action);
        context_menu.addSeparator();
    }

    if (emrview->selectionModel()->hasSelection())
    {
        context_menu.addAction(&emr_copy_action);
        context_menu.addSeparator();
    }

    context_menu.addAction(&emr_add_new_row_action);
    context_menu.addAction(&emr_delete_row_action);

    QAction *act = context_menu.exec(emrview->viewport()->mapToGlobal(pos));
    if (!act)
        return;

    if (act == &emr_copy_action)
        emrCopy();
    else if (act == &emr_add_new_row_action)
        emrmodel->insertRow(emrmodel->rowCount());
    else if (act == &emr_delete_row_action)
    {
        for (int i(list.last().row()); i >= list.first().row(); --i)
            emrmodel->removeRow(i);
    }
    else if (act == &emr_input_value_action)
    {
        bool isOk;
        QString newValue = QInputDialog::getText(this, tr("Новое значение"),
                                                tr("Введите значение:"), QLineEdit::Normal,
                                                QString(), &isOk);

        if (isOk && !newValue.isEmpty())
        {
            for (int i(0); i < list.size(); ++i)
                 emrmodel->setData(list.at(i), newValue.replace(QChar(','), QChar('.')).toDouble());
        }
    }
    else return;
}

void
MainWindow::emrCopy()
{
    QString copied, data;
    QModelIndexList list = emrview->selectionModel()->selectedIndexes();
    std::sort(list.begin(), list.end());

    for (int i(0); i < (list.size() - 1); ++i)
    {
        data = QString::number(list.at(i).data(Qt::EditRole).toDouble()).replace(QChar('.'), QChar(','));
        if (list.at(i).row() != list.at(i + 1).row())
            data.append('\n');
        else
            data.append('\t');

        copied.append(data);
    }

    copied.append(QString::number(list.last().data(Qt::EditRole).toDouble()).replace(QChar('.'), QChar(',')));
    clipboard->setText(copied);
}

void
MainWindow::on_emrCloseEditor_emitted(QWidget *editor)
{
    QModelIndex i = emrview->indexAt(editor->pos());
    double data = emrmodel->data(i, Qt::EditRole).toDouble();
    double prcnt = -0.3;

    switch (i.column())
    {
    case EmrColumns::RANGE:
        if (emrview->rowSpan(i.row(), EmrColumns::RANGE) == 1)
        {
            emrmodel->insertRows(i.row(), 2);
            for (int row(0); row < 3; ++row)
            {
                emrmodel->setData(emrmodel->index(i.row() + row, EmrColumns::RANGE), data);
                emrmodel->setData(emrmodel->index(i.row() + row, EmrColumns::PRESET), floor(data * (prcnt += 0.4)));
            }
        }
        else if (emrview->rowSpan(i.row(), EmrColumns::RANGE) == 3)
        {
            for (int row(0); row < 3; ++row)
                emrmodel->setData(emrmodel->index(i.row() + row, EmrColumns::PRESET), floor(data * (prcnt += 0.4)));
        } else
        {
            for (int row(i.row()); row < emrview->rowSpan(i.row(), i.column()); ++row)
                 emrmodel->setData(emrmodel->index(row, i.column()),
                                   emrmodel->data(emrmodel->index(i.row(), i.column()), Qt::EditRole));
        }
        break;
    case EmrColumns::RESOLUTION:
    case EmrColumns::ERROR:
    case EmrColumns::UNIT:
    case EmrColumns::ABSOLUTE:
        for (int row(i.row()); row < emrview->rowSpan(i.row(), i.column()); ++row)
             emrmodel->setData(emrmodel->index(row, i.column()),
                               emrmodel->data(emrmodel->index(i.row(), i.column()), Qt::EditRole));
        break;
    }
}

void
MainWindow::help()
{
    QFile file("help");
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::information(this, tr("Отсутствует файл справки"),
            file.errorString());
        return;
    }

    QString helpText;
    QTextStream in(&file);

    helpText = in.readAll();
    QMessageBox::information(this, tr("Помощь"), helpText.toLocal8Bit());

    file.close();
}

void
MainWindow::about()
{
    static const QString about =
    tr("<p align=\"center\"><b>Программа для обработки результатов измерений</b></p>"
       "<p align=\"center\">Версия %1<p>").arg(QCoreApplication::applicationVersion());

    QMessageBox::about(this, tr("О программе"), about);
}
