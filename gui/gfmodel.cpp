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

QString GFModel::polyIndexToString(size_t index)
{
    GfPoly *p = &field->tmp1;
    gf_poly_from_index(p, index, field->mod);

    if (index == 0)
        return "0";

    bool first = true;
    size_t i = p->len;
    QString str;

    while (i--) {
        auto n = p->at[i];

        if (n > 0) {
            if (!first)
                str += '+';
            if (n != 1 || i == 0)
                str += QString::number(n);
        } else {
            continue;
        }

        if (i == 1)
            str += 'x';
        else if (i != 0)
            str += "x^" + QString::number(i);

        first = false;
    }

    return str;
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
