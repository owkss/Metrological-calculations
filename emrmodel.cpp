#include "emrmodel.h"

EmrModel::EmrModel(QObject *parent)
          : QAbstractTableModel(parent)
{}

int
EmrModel::rowCount(const QModelIndex &/* UNUSED */) const
{
    return values.size();
}

int
EmrModel::columnCount(const QModelIndex &/* UNUSED */) const
{
    return N_COLUMNS;
}

QVariant
EmrModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch(role)
    {
        case Qt::DisplayRole:
            switch (index.column())
            {
            case RANGE:
                return values.at(index.row()).at(RANGE);
            case PRESET:
                return values.at(index.row()).at(PRESET);
            case RESOLUTION:
                return values.at(index.row()).at(RESOLUTION);
            case ERROR:
                return QString("±%1 %").arg(values.at(index.row()).at(ERROR));
            case UNIT:
                return QString(tr("%1 ед.м.р.")).arg(values.at(index.row()).at(UNIT));
            case ABSOLUTE:
                return QString("±%1").arg(values.at(index.row()).at(ABSOLUTE));
            case ABS_MIN:
                return values.at(index.row()).at(ABS_MIN);
            case MEASURE:
                return values.at(index.row()).at(MEASURE);
            case ABS_MAX:
                return values.at(index.row()).at(ABS_MAX);
            default:
                return QVariant();
            }
        case Qt::EditRole:
            switch (index.column())
            {
            case RANGE:
                return values.at(index.row()).at(RANGE);
            case PRESET:
                return values.at(index.row()).at(PRESET);
            case RESOLUTION:
                return values.at(index.row()).at(RESOLUTION);
             case ERROR:
                return values.at(index.row()).at(ERROR);
            case UNIT:
                return values.at(index.row()).at(UNIT);
            case ABSOLUTE:
                return values.at(index.row()).at(ABSOLUTE);
            case ABS_MIN:
                return values.at(index.row()).at(ABS_MIN);
            case MEASURE:
                return values.at(index.row()).at(MEASURE);
            case ABS_MAX:
                return values.at(index.row()).at(ABS_MAX);
            default:
                return QVariant();
        }

        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;
    }

    return QVariant();
}

bool
EmrModel::setData(const QModelIndex &index, const QVariant &_value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        switch (index.column())
        {
        case RANGE:
            values[index.row()][RANGE] = _value.toDouble();
            break;
        case PRESET:
            values[index.row()][PRESET] = _value.toDouble();
            break;
        case RESOLUTION:
            values[index.row()][RESOLUTION] = _value.toDouble();
            break;
        case ERROR:
            values[index.row()][ERROR] = _value.toDouble();
            break;
        case UNIT:
            values[index.row()][UNIT] = _value.toDouble();
            break;
        case ABSOLUTE:
            values[index.row()][ABSOLUTE] = _value.toDouble();
            break;
        case ABS_MIN:
            values[index.row()][ABS_MIN] = _value.toDouble();
            break;
        case MEASURE:
            values[index.row()][MEASURE] = _value.toDouble();
            break;
        case ABS_MAX:
            values[index.row()][ABS_MAX] = _value.toDouble();
            break;
        default:
            return false;
        }

        emit dataChanged(index, index);
        return true;
    }
    return false;
}

QVariant
EmrModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
            case RANGE:
                 return tr("Диапазон");
            case PRESET:
                 return tr("Установлено");
            case RESOLUTION:
                return tr("Разрешение");
            case ERROR:
                return tr("Погрешность");
            case UNIT:
                return tr("Е.М.Р");
            case ABSOLUTE:
                return tr("Абс. погр-ть");
            case ABS_MIN:
                return tr("Мин. порог");
            case MEASURE:
                return tr("Измеренное");
            case ABS_MAX:
                return tr("Макс. порог");
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
EmrModel::flags(const QModelIndex &index) const
{
    return index.isValid() ? (QAbstractTableModel::flags(index) | Qt::ItemIsEditable)
                           : QAbstractTableModel::flags(index);
}

bool
EmrModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (count < 1 || row < 0)
        return false;

    beginInsertRows(parent, row, row + count - 1);

    for (int i(0); i < count; ++i)
        values.insert(row + i, QVector<double>(N_COLUMNS));

    endInsertRows();

    return true;
}

bool
EmrModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (count < 1 || row < 0)
        return false;

    beginRemoveRows(parent, row - count + 1, row);

    for (int i(0); i < count; ++i)
        values.remove(row - i);

    endRemoveRows();

    return true;
}

bool
EmrModel::clear()
{
    return removeRows(rowCount() - 1, rowCount()) ? true : false;
}
