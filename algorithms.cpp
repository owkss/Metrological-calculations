#include "algorithms.h"
#include "tablemodel.h"

#include <QtMath>
#include <QTableView>

Calculations::Calculations(const QTableView *table, const QModelIndex &index)
{
    TableModel *model = dynamic_cast<TableModel*>(table->model());

    int span = table->rowSpan(index.row(), model->PRES);
    double pres, meas;

    // Первая  строка
    int topRow = getTopRow(table, index.row());

    pres = model->data(model->index(topRow, model->PRES), Qt::DisplayRole).toDouble();
    meas = model->data(model->index(index.row(), model->MEAS), Qt::DisplayRole).toDouble();

    // Абсолютная погрешность
    abslt = meas - pres;

    // Относительная погрешность
    err = abslt / pres * 100.0;

    // Среднее арифметическое
    for(int i(0); i < span; ++i)
        avrg += model->data(model->index(topRow + i, model->MEAS), Qt::DisplayRole).toDouble();
    avrg /= span;

    // Дисперсия
    for(int i(0); i < span; ++i)
        disp += qPow((model->data(model->index(topRow + i, model->MEAS), Qt::DisplayRole).toDouble() - avrg), 2);
    disp /= (span - 1.5);

    // Среднеквадратичное отклонение
    dev = qSqrt(disp);

    // Критерий Граббса
    for(int i(0); i < span; ++i)
    {
        double g = model->data(model->index(topRow + i, model->MEAS), Qt::DisplayRole).toDouble();
        g = abs(g - avrg) / dev;
        grubbs.push_back(g);
    }
}

Calculations::~Calculations()
{
    grubbs.clear();
}

int
getTopRow(const QTableView *table, const int row)
{
    TableModel *model = dynamic_cast<TableModel*>(table->model());
    int topRow(0);
    while (topRow + table->rowSpan(topRow, model->PRES) <= row)
           topRow += table->rowSpan(topRow, model->PRES);

    return topRow;
}

double
roundV(double value, const int &prcsn)
{
    double result(0.0);
    result = floor(value * prcsn + 0.5) / prcsn;

    return result;
}
