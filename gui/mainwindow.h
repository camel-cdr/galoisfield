#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include "gfmodel.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateField(unsigned n, GfPoly *irreducible);


private:
    static QString polyToString(GfPoly p);
    bool stringToInt(QString str, unsigned *x);
    bool editIrreducible(unsigned n, QString msg, QString defaultVale);

private slots:
    void on_pushButton_clicked();

    void on_irreducibleButton_clicked();

    void on_calcButton_clicked();

private:
    Ui::MainWindow *ui;
    GFModel *modelAdd;
    GFModel *modelMul;
    GField field = {};
};
#endif // MAINWINDOW_H
