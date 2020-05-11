#include "graphic.h"

#include <QTableView>
#include <QGridLayout>

Graphic::Graphic(QWidget *parent) : view(qobject_cast<QTableView*>(parent))
{
    model = dynamic_cast<TableModel*>(view->model());

    this->setAttribute(Qt::WA_QuitOnClose);
    this->setWindowTitle(QObject::tr("Дисперсия случайной величины"));
    this->setWindowIcon(QIcon(":/images/graphic.png"));

    graphic = new QwtPlot(this);
    legend = new QwtLegend(this);
    graphic->setCanvasBackground(Qt::white);
    graphic->setAxisTitle(QwtPlot::yLeft, QObject::tr("Значение"));
    graphic->setAxisTitle(QwtPlot::xBottom, QObject::tr("Измерения"));
    graphic->setAxisScale(QwtPlot::xBottom, 0, 10);
    graphic->insertLegend(legend);

    grid = new QwtPlotGrid();
    QPen majorPen(Qt::gray, 1, Qt::DotLine);
    grid->setMajorPen(majorPen);
    grid->attach(graphic);

    QBrush fill = Qt::cyan;
    QPen pen(Qt::blue, 2);
    QSize size(8, 8);
    symbol = new QwtSymbol(QwtSymbol::Ellipse, fill,
                           pen, size);

    list = view->selectionModel()->selectedIndexes();
    for (int i(0); i < list.size(); ++i)
        measures << QPointF(i + 1, model->index(list.at(i).row(), model->MEAS).data().toDouble());

    higDev << QPointF(0, model->index(list.first().row(), model->AVRG).data().toDouble()
                      + model->index(list.first().row(), model->DEV).data().toDouble())
           << QPointF(list.size(), model->index(list.first().row(), model->AVRG).data().toDouble()
                      + model->index(list.first().row(), model->DEV).data().toDouble());
    lowDev << QPointF(0, model->index(list.first().row(), model->AVRG).data().toDouble()
                      - model->index(list.first().row(), model->DEV).data().toDouble())
           << QPointF(list.size(), model->index(list.first().row(), model->AVRG).data().toDouble()
                       - model->index(list.first().row(), model->DEV).data().toDouble());
    avrg   << QPointF(0, model->index(list.first().row(), model->AVRG).data().toDouble())
           << QPointF(list.size(), model->index(list.first().row(), model->AVRG).data().toDouble());

    measureCurve = new QwtPlotCurve();
    measureCurve->setTitle(QObject::tr("Измеренные значения"));
    measureCurve->setPen(Qt::darkMagenta, 5);
    measureCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    measureCurve->setSymbol(symbol);
    measureCurve->setSamples(measures);
    measureCurve->attach(graphic);

    highDeviation = new QwtPlotCurve();
    highDeviation->setTitle(QObject::tr("Верхнее отклонение"));
    highDeviation->setPen(Qt::red, 2);
    highDeviation->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    highDeviation->setSamples(higDev);
    highDeviation->attach(graphic);

    lowDeviation = new QwtPlotCurve();
    lowDeviation->setTitle(QObject::tr("Нижнее отклонение"));
    lowDeviation->setPen(Qt::darkRed, 2);
    lowDeviation->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    lowDeviation->setSamples(lowDev);
    lowDeviation->attach(graphic);

    average = new QwtPlotCurve();
    average->setTitle(QObject::tr("Среднее арифметическое"));
    average->setPen(Qt::darkGreen, 2);
    average->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    average->setSamples(avrg);
    average->attach(graphic);

    magnifier = new QwtPlotMagnifier(graphic->canvas());
    magnifier->setMouseButton(Qt::MidButton);

    panner = new QwtPlotPanner(graphic->canvas());
    panner->setMouseButton(Qt::RightButton);

    picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                               QwtPlotPicker::CrossRubberBand,
                               QwtPicker::ActiveOnly, graphic->canvas());
    QColor rubberPen(Qt::red);
    QColor trackerPen(Qt::black);
    picker->setRubberBandPen(rubberPen);
    picker->setTrackerPen(trackerPen);
    picker->setStateMachine(new QwtPickerDragPointMachine);

    layout = new QGridLayout(this);
    layout->addWidget(graphic);
}

Graphic::~Graphic()
{
    delete grid;
    delete measureCurve;
    delete highDeviation;
    delete lowDeviation;
    delete average;
    delete magnifier;
    delete panner;
    delete picker;
}
