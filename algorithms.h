#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <QVector>
#include <QModelIndex>

QT_BEGIN_NAMESPACE
class QTableView;
QT_END_NAMESPACE

struct Calculations
{
    QVector<double> grubbs;
    double err{0}, abslt{0}, avrg{0}, dev{0}, disp{0};

    explicit Calculations(const QTableView*, const QModelIndex&);
    ~Calculations();
};

int getTopRow(const QTableView *, const int row);
double roundV(double, const int&);

#endif // ALGORITHMS_H
