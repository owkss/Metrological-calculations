#ifndef EMRMODEL_H
#define EMRMODEL_H

#include <QVector>
#include <QAbstractTableModel>

enum EmrColumns
{ RANGE = 0, PRESET, RESOLUTION, ERROR, UNIT, ABSOLUTE, ABS_MIN, MEASURE, ABS_MAX, N_COLUMNS };

class EmrModel : public QAbstractTableModel
{

public:
    EmrModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &_value, int role = Qt::EditRole) override;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent  = QModelIndex()) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool clear();

private:
    QVector<QVector<double>> values;
};

#endif // EMRMODEL_H
