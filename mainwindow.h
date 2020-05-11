#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFile>
#include <QMainWindow>

//#include <tbb/task_scheduler_init.h>

QT_BEGIN_NAMESPACE
class EmrModel;
class QSpinBox;
class QComboBox;
class QTextEdit;
class QLineEdit;
class QTabWidget;
class QTableView;
class QStandardItemModel;
class QDoubleSpinBox;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    //tbb::task_scheduler_init init;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QTableView * createTable();
    void createBars();
    void createMenus();
    void createDockWidget();

    void about();
    void addRowLow(const QModelIndex&);
    void addRowTop(const QModelIndex&);
    void changeData(const QModelIndex&);
    void convertValues(int);
    void copy();
    void customMenuRequested(const QPoint);
    void decrementPrecision();
    void emrChangeData(const QModelIndex&);
    void emrCopy();
    void emrCustomMenuRequested(const QPoint);
    void help();
    void incrementPrecision();
    void open();
    void readFromFile(QString &);
    void ratioValues(int);
    void writeAsDocument();
    void writeToFile();

    void save_as_xlsx();

private slots:
    void on_addTabAct_triggered();
    void on_cleanItemsAct_triggered();
    void on_clearAct_triggered();
    void on_closeEditor_emitted(QWidget*);
    void on_deleteRowAct_triggered();
    void on_editTabAct_triggered();
    void on_emrCloseEditor_emitted(QWidget*);
    void on_measureCountSpinBox_valueChanged(int);
    void on_presetAdd_returnPressed();
    bool on_quitAppAct_triggered();
    void on_setLimitAct_triggered();
    void on_setSpanAct_triggered();
    void on_showGraphicAct_triggered();
    void on_resetSpanAct_triggered();
    void on_tabClose_requested(int);
    void on_tabWidget_currentChanged(int);

private:
    int prcsn = 100000; /* Точность округления */

    QTabWidget *tabWidget = nullptr;
    QClipboard *clipboard = nullptr;
    QLineEdit *presetAdd = nullptr;
    QSpinBox *measureCountSpinBox = nullptr;

    QWidget *convertWidget = nullptr;
    QComboBox *convertBox = nullptr;
    QLineEdit *convertInput = nullptr;
    QTextEdit *convertResult = nullptr;
    QAction *showConvertWidgetAct = nullptr;

    QWidget *ratioWidget = nullptr;
    QComboBox *ratioBox = nullptr;
    QLineEdit *firstRatioInput = nullptr;
    QLineEdit *secondRatioInput = nullptr;
    QLineEdit *ratioResult = nullptr;

    QDockWidget *dock = nullptr;
    QTableView *emrview = nullptr;
    EmrModel *emrmodel = nullptr;

    QMenu *fileMenu = nullptr;
    QMenu *editMenu = nullptr;
    QMenu *aboutMenu = nullptr;

    QToolBar *leftToolBar = nullptr;
    QToolBar *controlToolBar = nullptr;
    QToolBar *calcToolBar = nullptr;

    QAction *openTableAct = nullptr;
    QMenu *save_menu = nullptr;
    QAction *saveTableAct = nullptr;
    QAction *saveAsDocumentAct = nullptr;
    QAction *save_as_xlsx_action = nullptr;
    QAction *quitAppAct = nullptr;

    QAction *helpAct = nullptr;
    QAction *aboutAct = nullptr;
    QAction *aboutQtAct = nullptr;

    QAction *addTabAct = nullptr;
    QAction *deleteTabAct = nullptr;
    QAction *editTabAct = nullptr;
    QAction *clearAct = nullptr;
    QAction *decimaMoreAct = nullptr;
    QAction *decimaLessAct = nullptr;
    QAction *showGraphicAct = nullptr;
    QAction *returnAct = nullptr;
    QAction *showEmrTable = nullptr;

    void saveTempFile();
    void openTempFile();

    QFile *temp = nullptr;
    QString tabHeaderName;

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // MAINWINDOW_H
