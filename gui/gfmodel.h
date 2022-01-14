#ifndef GFMODEL_H
#define GFMODEL_H

#include <QAbstractItemModel>
#include <QFont>

extern "C" {
#include "../gf.h"
}


class GFModel : public QAbstractItemModel
{
public:
    explicit GFModel(size_t (*op)(GField *f, size_t i, size_t j), QString opStr, GField *field = nullptr);
    void setField(GField *field);

    QString opStr;
    size_t (*op)(GField *f, size_t i, size_t j);
    GField *field;
    QFont font;


    // QAbstractItemModel interface
public:
    QString polyIndexToString(size_t index);
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
};

#endif // GFMODEL_H
