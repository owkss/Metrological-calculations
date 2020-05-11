#include "mainwindow.h"

#include <QApplication>
#include <QThreadPool>

int
main(int argc, char *argv[])
{
    QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount() > 2 ? QThread::idealThreadCount() : 2);
    QApplication app(argc, argv);

    QApplication::setWindowIcon(QIcon(":/images/icon.png"));
    QCoreApplication::setOrganizationName("OWKSS");
    QCoreApplication::setApplicationName("Error&Uncertainty");
    QCoreApplication::setApplicationVersion("3.1.0");

    MainWindow mw;
    mw.setWindowTitle(QObject::tr("Обработка результатов измерений"));
    mw.resize(1100, 700);
    mw.show();

    return app.exec();
}
