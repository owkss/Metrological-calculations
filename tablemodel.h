#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QMap>
#include <QAbstractTableModel>

struct Columns
{
    int measures = 1;
    double limit{0}, preset{0}, measure{0}, error{0}, abs{0},
           average{0}, deviation{0}, dispersion{0}, grubbs{0};

    bool operator==(const Columns &c);
    bool operator<(const Columns &c);
};

class TableModel : public QAbstractTableModel
{
public:
    enum
    { LIM = 0, PRES, MEAS, ERR, ABS, AVRG, DEV, DISP, GRUBBS, N_COLUMNS };

    QMap<int, double> c_grubbs;

    TableModel(QObject *parent = nullptr);
    void createGrubbsTable();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &_value, int role = Qt::EditRole) override;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent  = QModelIndex()) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setMeasuresCount(const int row, const int count);
    int getMeasuresCount(const int row);

    void setLimit(const int row, const double lim);
    double getLimit(const int row);

    void refresh() { beginResetModel(); endResetModel(); }
    bool clear();

private:
    QVector<Columns> values;
};

#endif // TABLEMODEL_H
