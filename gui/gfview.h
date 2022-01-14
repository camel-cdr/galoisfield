#ifndef GFVIEW_H
#define GFVIEW_H

#include <QTableView>
#include <QHeaderView>
#include <QLabel>
#include "gfmodel.h"

class GFView : public QTableView
{
public:
    GFView(QWidget *parent);
    void init(GFModel *model, QLabel *lbl);
    void setWidth(int width);

private slots:
   void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);


private:
   QLabel *lbl;
   GFModel *gfmodel;
};

#endif // GFVIEW_H
