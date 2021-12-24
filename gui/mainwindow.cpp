#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <math.h>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , modelAdd(new GFModel(gfield_add))
    , modelMul(new GFModel(gfield_mul))
{
    ui->setupUi(this);
    ui->input->setValidator(new QIntValidator(0, 9999999, this));

    ui->opCombo->addItem("+");
    ui->opCombo->addItem("-");
    ui->opCombo->addItem("*");
    ui->opCombo->addItem("/");

    ui->additionTable->setModel(modelAdd);
    ui->additionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->additionTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    ui->multiplicationTable->setModel(modelMul);
    ui->multiplicationTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->multiplicationTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateField(unsigned n, GfPoly *irreducible)
{
    auto res = gfield_init(&field, n, irreducible);
    if (res != GF_INIT_SUCCESS) {
        if (res == GF_INIT_DOESNT_EXIST)
            QMessageBox::warning(this, "Invalid input", QString("GF(%1) doesn't exist").arg(n));
        else if (res == GF_INIT_IRREDUCIBLE_IS_ZERO)
            QMessageBox::warning(this, "Invalid input", QString("The irreducible polynomial can't be zero").arg(n));
        else
            QMessageBox::warning(this, "Invalid input", QString("GF(%1) isn't supported, because it's to big").arg(n));
        ui->input->setText(QString::number(field.n));
        return;
    }

    ui->irreducibleLine->setText(polyToString(field.irreducible));

    ui->irreducibleButton->setEnabled(true);
    ui->irreducibleLable->setEnabled(true);
    ui->irreducibleLine->setEnabled(true);
    ui->calcBox->setEnabled(true);
    ui->calcBox->setTitle(QString("Calculate with GF(") + QString::number(field.n) + "=" + QString::number(field.mod.mod) + "^" + QString::number(field.power) + "):");


    ui->additionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->multiplicationTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    {
        auto width = ui->additionTable->horizontalHeader()->fontMetrics().averageCharWidth() * ceil(log10(n - 1) + 3);
        ui->additionTable->horizontalHeader()->setMaximumSectionSize(width);
        ui->additionTable->horizontalHeader()->setMinimumSectionSize(width);
        ui->additionTable->horizontalHeader()->setDefaultSectionSize(width);
    }
    {
        auto width = ui->multiplicationTable->horizontalHeader()->fontMetrics().averageCharWidth() * ceil(log10(n - 1) + 3);
        ui->multiplicationTable->horizontalHeader()->setMaximumSectionSize(width);
        ui->multiplicationTable->horizontalHeader()->setMinimumSectionSize(width);
        ui->multiplicationTable->horizontalHeader()->setDefaultSectionSize(width);
    }
    ui->additionTable->verticalHeader()->setDefaultSectionSize(QApplication::font().pointSize());
    ui->multiplicationTable->verticalHeader()->setDefaultSectionSize(QApplication::font().pointSize());

    modelAdd->setField(&field);
    modelMul->setField(&field);
}

QString MainWindow::polyToString(GfPoly p)
{
    if (p.len == 0)
        return "0";
    QString str = QString("%1").arg(p.at[p.len - 1]);
    for (size_t i = p.len - 1; i--; )
        str += QString(" %1").arg(p.at[i]);
    return str;
}

bool MainWindow::stringToInt(QString str, unsigned *x)
{
    bool ok = true;
    unsigned n = str.toUInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Invalid input", "Couldn't parse text as Integer");
        return false;
    }
    *x = n;
    return true;
}

bool MainWindow::editIrreducible(unsigned n, QString msg, QString defaultVal)
{
    GfPoly p = {};
    bool ok;
    QString text = QInputDialog::getText(this, "Input Irreducible Polynomial",
                                         msg,
                                         QLineEdit::Normal, defaultVal, &ok);
    if (!ok || text.isEmpty())
        goto err;

    {
        auto arr = text.trimmed().split(" ");
        gf_poly_setlen(&p, arr.length());
        size_t i = arr.length() - 1;
        for (auto x : arr) {
            unsigned n = x.toUInt(&ok);
            if  (!ok) {
                goto err;
            }
            p.at[i--] = n;
        }
    }

    updateField(n, &p);
    gf_poly_free(&p);
    return true;

err:
    gf_poly_free(&p);
    QMessageBox::warning(this, "Invalid input", "Polynomial couldn't be parsed, expected space seperated multiplicands");
    return false;
}

void MainWindow::on_pushButton_clicked()
{
    unsigned n;
    if (!stringToInt(ui->input->displayText(), &n)) {
        ui->input->setText(QString::number(field.n));
        return;
    }

    if (n > 10000) {
        if (!editIrreducible(n, "Input Irreducible Polynomial, since calculating it would take to long:", ""))
            ui->input->setText(QString::number(field.n));
    } else {
        updateField(n, nullptr);
    }
}

void MainWindow::on_irreducibleButton_clicked()
{
    editIrreducible(field.n, "Input Irreducible Polynomial", ui->irreducibleLine->text());
}


void MainWindow::on_calcButton_clicked()
{
    unsigned l, r, res;

    if (!stringToInt(ui->calcLhs->text(), &l))
        return;
    if (!stringToInt(ui->calcRhs->text(), &r))
        return;

    switch (ui->opCombo->currentText()[0].toLatin1()) {
    case '+':
        res = gfield_add(&field, l, r);
        break;
    case '-':
        res = gfield_sub(&field, l, r);
        break;
    case '*':
        res = gfield_mul(&field, l, r);
        break;
    case '/':
        res = gfield_div(&field, l, r);
        break;
    default:
        return;
    }
    ui->calcOutput->setText(QString::number(res));
}

