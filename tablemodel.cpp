#include "tablemodel.h"

#include <QtMath>
#include <QBrush>

TableModel::TableModel(QObject *parent)
          : QAbstractTableModel(parent)
{
	createGrubbsTable();
}

int
TableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return values.size();
}

int
TableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return N_COLUMNS;
}

QVariant
TableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    QString show;
    switch(role)
    {
        case Qt::DisplayRole:
        switch (index.column())
        {
            case LIM:
                 show = QString("±%1 %").arg(values.at(index.row()).limit);
                 return show;
            case PRES:
                 return values.at(index.row()).preset;
            case MEAS:
                 return values.at(index.row()).measure;
            case ERR:
                 show = QString("%1 %").arg(values.at(index.row()).error);
                 return show;
            case ABS:
                 return values.at(index.row()).abs;
            case AVRG:
                return values.at(index.row()).average;
            case DEV:
                return values.at(index.row()).deviation;
            case DISP:
                return values.at(index.row()).dispersion;
            case GRUBBS:
                return values.at(index.row()).grubbs;
            default:
                 return QVariant();
        }

        case Qt::EditRole:
        switch (index.column())
        {
            case LIM:
                return values.at(index.row()).limit;
            case PRES:
                return values.at(index.row()).preset;
            case MEAS:
                return values.at(index.row()).measure;
            case ERR:
                return values.at(index.row()).error;
            case ABS:
                return values.at(index.row()).abs;
            case AVRG:
                return values.at(index.row()).average;
            case DEV:
                return values.at(index.row()).deviation;
            case DISP:
                return values.at(index.row()).dispersion;
            case GRUBBS:
                return values.at(index.row()).grubbs;
            default:
                return QVariant();
        }

        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;

        case Qt::BackgroundRole:
        switch(index.column())
        {
            case ERR:
                if ((values.at(index.row()).error >= values.at(index.row()).limit) ||
                    (values.at(index.row()).error <= (-1.0 * values.at(index.row()).limit)))
                        return QBrush(Qt::red);
            break;

            case GRUBBS:
                if ((values.at(index.row()).measures >= 3) &&
                    (values.at(index.row()).grubbs >= c_grubbs[values.at(index.row()).measures]))
                        return QBrush(Qt::red);
            break;
        }
        break;

        case Qt::ToolTipRole:
        switch(index.column())
        {
            case LIM:
                show = QString("U=%1dB; P=%2dB\nU=%3dB; P=%4dB")
                       .arg(20*log10(1.0 + ((-1.0 * values.at(index.row()).limit) / 100.0)))
                       .arg(10*log10(1.0 + ((-1.0 * values.at(index.row()).limit) / 100.0)))
                       .arg(20*log10(1.0 + (values.at(index.row()).limit / 100.0)))
                       .arg(10*log10(1.0 + (values.at(index.row()).limit / 100.0)));
                return show;
            case MEAS:
                show = QString(tr("Δ: %1 - %2"))
                       .arg(values.at(index.row()).preset - (values.at(index.row()).preset * values.at(index.row()).limit / 100))
                       .arg(values.at(index.row()).preset + (values.at(index.row()).preset * values.at(index.row()).limit / 100));
                return show;
            case ERR:
                show = QString("δU=%1dB; δP=%2dB")
                       .arg(20*log10(1 + (values.at(index.row()).error / 100)))
                       .arg(10*log10(1 + (values.at(index.row()).error / 100)));
                return show;
            case AVRG:
                show = QString(tr("Оценка величины: %1"))
                       .arg(values.at(index.row()).deviation / qSqrt(values.at(index.row()).measures));
                return show;
            case GRUBBS:
                if ((values.at(index.row()).measures >= 3) && (index.column() == GRUBBS))
                {
                    show = QString("G(%1) = %2")
                           .arg(values.at(index.row()).measures)
                           .arg(c_grubbs[values.at(index.row()).measures]);
                    return show;
                }
        }
    }

    return QVariant();
}

bool
TableModel::setData(const QModelIndex &index, const QVariant &_value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        switch (index.column())
        {
            case LIM:
                 values[index.row()].limit = abs(_value.toDouble());
                 break;
            case PRES:
                 values[index.row()].preset = _value.toDouble();
                 break;
            case MEAS:
                 values[index.row()].measure = _value.toDouble();
                 break;
            case ERR:
                 values[index.row()].error = _value.toDouble();
                 break;
            case ABS:
                 values[index.row()].abs = _value.toDouble();
                 break;
            case AVRG:
                values[index.row()].average = _value.toDouble();
                break;
            case DEV:
                values[index.row()].deviation = _value.toDouble();
                break;
            case DISP:
                values[index.row()].dispersion = _value.toDouble();
                break;
            case GRUBBS:
                values[index.row()].grubbs = _value.toDouble();
                break;
            default:
                 return false;
        }

        emit dataChanged(index, index, {role});

        return true;
    }

    return false;
}

QVariant
TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
            case LIM:
                 return tr("Доп. предел");
            case PRES:
                return tr("Уст. значение");
            case MEAS:
                return tr("Изм. значение");
            case ERR:
                return tr("Отн. погр-ть");
            case ABS:
                return tr("Абс. погр-ть");
            case AVRG:
                return tr("Среднее арифм.");
            case DEV:
                return tr("Среднеквадр. откл-е");
            case DISP:
                return tr("Дисперсия");
            case GRUBBS:
                return tr("Критерий Граббса");
            default:
                return QVariant();
        }
    }

    if (orientation == Qt::Vertical)
    {
        return section + 1;
    }

    return QVariant();
}

Qt::ItemFlags
TableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    switch(index.column())
    {
        case LIM:
        case PRES:
        case MEAS:
            return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
        default:
            return QAbstractTableModel::flags(index) | Qt::ItemIsEnabled;
    }
}

void
TableModel::setMeasuresCount(const int row, const int count)
{
    values[row].measures = count;
}

int
TableModel::getMeasuresCount(const int row)
{
    return values.at(row).measures;
}

void
TableModel::setLimit(const int row, const double lim)
{
    values[row].limit = lim;
}

double
TableModel::getLimit(const int row)
{
    return values.at(row).limit;
}

bool
TableModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (count < 1 || row < 0)
        return false;

    beginInsertRows(parent, row, row + count - 1);

    for (int i(0); i < count; ++i)
        values.insert(row + i, Columns());

    endInsertRows();

    return true;
}

bool
TableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row - count + 1, row);

    for (int i(0); i < count; ++i)
        values.remove(row - i);

    endRemoveRows();

    return true;
}

bool
TableModel::clear()
{
    return removeRows(rowCount() - 1, rowCount()) ? true : false;
}

void
TableModel::createGrubbsTable()
{
    c_grubbs[3] = 1.155; c_grubbs[4] = 1.481;
    c_grubbs[5] = 1.715; c_grubbs[6] = 1.887;
    c_grubbs[7] = 2.020; c_grubbs[8] = 2.1260;
    c_grubbs[9] = 2.215; c_grubbs[10] = 2.290;
    c_grubbs[11] = 2.355; c_grubbs[12] = 2.412;
    c_grubbs[13] = 2.462; c_grubbs[14] = 2.507;
    c_grubbs[15] = 2.549; c_grubbs[16] = 2.585;
    c_grubbs[17] = 2.620; c_grubbs[18] = 2.651;
    c_grubbs[19] = 2.681; c_grubbs[20] = 2.709;
    c_grubbs[21] = 2.733; c_grubbs[22] = 2.758;
    c_grubbs[23] = 2.781; c_grubbs[24] = 2.876;
    c_grubbs[25] = 2.822; c_grubbs[26] = 2.841;
    c_grubbs[27] = 2.859; c_grubbs[28] = 2.876;
    c_grubbs[29] = 2.893; c_grubbs[30] = 2.908;
}

bool
Columns::operator==(const Columns &c)
{
    return qFuzzyCompare(this->limit, c.limit) && qFuzzyCompare(this->preset, c.preset) && qFuzzyCompare(this->measure, c.measure)
           && qFuzzyCompare(this->error, c.error) && qFuzzyCompare(this->abs, c.abs)
           && qFuzzyCompare(this->average, c.average) && qFuzzyCompare(this->deviation, c.deviation)
           && qFuzzyCompare(this->dispersion, c.dispersion) && qFuzzyCompare(this->grubbs, c.grubbs);
}

bool
Columns::operator<(const Columns &c)
{
    if (this->preset < c.preset && this->measure < c.measure && this->error < c.error
                                && this->abs < c.abs && this->average < c.average && this->deviation < c.deviation
                                && this->dispersion < c.dispersion && this->grubbs < c.grubbs)
        return true;
    else
        return false;
}
