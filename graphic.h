#ifndef GRAPHIC_H
#define GRAPHIC_H

#include "tablemodel.h"

#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>

#include <QDialog>

QT_BEGIN_NAMESPACE
class QTableView;
class QGridLayout;
QT_END_NAMESPACE

class Graphic : public QDialog
{
public:
    explicit Graphic(QWidget *parent);
    ~Graphic();

private:
    QTableView *view = nullptr;
    TableModel *model = nullptr;

    QModelIndexList list;
    QGridLayout *layout = nullptr;

    QPolygonF higDev, lowDev, avrg, measures;

    QwtPlot *graphic = nullptr;
    QwtPlotGrid *grid = nullptr;
    QwtSymbol *symbol = nullptr;
    QwtLegend *legend = nullptr;

    QwtPlotCurve *measureCurve = nullptr;
    QwtPlotCurve *highDeviation = nullptr;
    QwtPlotCurve *lowDeviation = nullptr;
    QwtPlotCurve *average = nullptr;

    QwtPlotMagnifier *magnifier = nullptr;

    QwtPlotPanner *panner = nullptr;
    QwtPlotPicker *picker = nullptr;
};

#endif // GRAPHIC_H
