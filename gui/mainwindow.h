#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include "gfmodel.h"
#include "gfview.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setEditMode(bool edit);
    void updateField(GfPoly *irreducible = nullptr);
    static QString polyToString(GfPoly p);
    bool stringToInt(QString str, unsigned *x);

private slots:
    void on_updateButton_clicked();
    void on_editButton_clicked();
    void on_calcButton_clicked();

private:
    bool editMode = false;

    Ui::MainWindow *ui;
    GFModel *modelAdd;
    GFModel *modelMul;
    GField field = {};
};
#endif // MAINWINDOW_H
