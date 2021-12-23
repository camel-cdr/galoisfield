#include "gfmodel.h"
#include <unistd.h>

GFModel::GFModel(size_t (*op)(GField *f, size_t i, size_t j), GField *field)
    : op(op)
    , field(field)
    , font("monospace")
{
}

void GFModel::setField(GField *field)
{
    beginResetModel();
    this->field = field;
    endResetModel();
}

QModelIndex GFModel::index(int row, int column, const QModelIndex &parent) const
{
    return createIndex(row, column, nullptr);
}

QModelIndex GFModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

int GFModel::rowCount(const QModelIndex &parent) const
{
    if (!field)
        return 0;
    return field->n;
}

int GFModel::columnCount(const QModelIndex &parent) const
{
    if (!field)
        return 0;
    return field->n;
}


QVariant GFModel::data(const QModelIndex &index, int role) const
{
    if (!field)
        return QVariant();

    switch(role) {
    case Qt::DisplayRole:
        return QVariant((unsigned)op(field, index.column(), index.row()));
    case Qt::TextAlignmentRole:
        return Qt::AlignCenter;
    case Qt::FontRole:
        return font;
    default:
        return QVariant();
    }
}

QVariant GFModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch(role) {
    case Qt::DisplayRole:
        return QVariant(section);
    case Qt::TextAlignmentRole:
        return Qt::AlignCenter;
    case Qt::FontRole:
        return font;
    default:
        return QVariant();
    }
}

Qt::ItemFlags GFModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren;
}
