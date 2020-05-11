#include "delegate.h"

#include <QLineEdit>

Delegate::Delegate(QWidget *parent) : QStyledItemDelegate(parent)
{}

QWidget*
Delegate::createEditor(QWidget *parent,
                       const QStyleOptionViewItem &/* UNUSED */,
                       const QModelIndex &/* UNUSED */) const
{
    QLineEdit *editor = new QLineEdit(parent);
    return editor;
}

void
Delegate::setEditorData(QWidget *editor,
                        const QModelIndex &index) const
{
    QVariant value = index.data(Qt::EditRole);
    static_cast<QLineEdit*>(editor)->setText(value.toString());
}

void
Delegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                       const QModelIndex &index) const
{
    QVariant value = static_cast<QLineEdit*>
                     (editor)->text().replace(QChar(','), QChar('.'));
    model->setData(index, value, Qt::EditRole);
}

QString
Delegate::displayText(const QVariant &value, const QLocale &/* UNUSED */) const
{
    return value.toString();
}
