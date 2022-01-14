#include "gfview.h"

GFView::GFView(QWidget *parent)
    : QTableView(parent)
{
}

void GFView::init(GFModel *model, QLabel *lbl)
{
    this->lbl = lbl;
    gfmodel = model;
    setModel(model);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    setSelectionMode(SingleSelection);
}

void GFView::setWidth(int width)
{
    width *= horizontalHeader()->fontMetrics().averageCharWidth();
    horizontalHeader()->setMaximumSectionSize(width);
    horizontalHeader()->setMinimumSectionSize(width);
    horizontalHeader()->setDefaultSectionSize(width);

    verticalHeader()->setDefaultSectionSize(verticalHeader()->font().pointSize());
    verticalHeader()->setDefaultSectionSize(verticalHeader()->font().pointSize());
}

void GFView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QString str;
    if (selected.count() != 1)
        return;
    auto sel = selected.indexes()[0];
    str += "(";
    str += gfmodel->polyIndexToString(sel.row());
    str += ") " + gfmodel->opStr + " (";
    str += gfmodel->polyIndexToString(sel.column());
    str += ") = ";
    str += gfmodel->polyIndexToString(sel.data().toUInt());
    lbl->setText(str);
}
