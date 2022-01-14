#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <cmath>


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


void MainWindow::setEditMode(bool edit)
{
    editMode = edit;
    ui->input->setDisabled(edit);
    ui->editButton->setDisabled(edit);
    ui->calcBox->setDisabled(edit);
    ui->tabWidget->setDisabled(edit);
    ui->irreducibleLine->setEnabled(true);
    ui->irreducibleLine->setReadOnly(!edit);
    ui->irreducibleLine->setFocus();
    ui->irreducibleLine->setSelection(0, ui->irreducibleLine->text().length());
}

void MainWindow::updateField(GfPoly *irreducible)
{
    unsigned n;
    if (!stringToInt(ui->input->displayText(), &n)) {
        ui->input->setText(QString::number(field.n));
        return;
    }


    size_t mod = 0, power = 0;
    if (!gf_factor(n, &mod, &power)) {
        QMessageBox::warning(this, "Invalid input", QString("GF(%1) doesn't exist").arg(n));
        return;
    }

    if (/* n > 10000 && */ !editMode) {
        setEditMode(true);
        return;
    }


    auto res = gfield_init(&field, n, irreducible);

    /* setup the GUI */

    if (res != GF_INIT_SUCCESS) {
        if (res == GF_INIT_DOESNT_EXIST)
            QMessageBox::warning(this, "Invalid input", QString("GF(%1) doesn't exist").arg(n));
        else if (res == GF_INIT_IRREDUCIBLE_INVALID)
            QMessageBox::warning(this, "Invalid input", QString("The irreducible polynomial is invalid, make sure it isn't zero").arg(n));
        else
            QMessageBox::warning(this, "Invalid input", QString("GF(%1) isn't supported, because it's to big").arg(n));
        ui->input->setText(QString::number(field.n));
        ui->input->setFocus();
        ui->input->setSelection(0, ui->input->text().length());
        return;
    }

    setEditMode(false);

    ui->irreducibleLine->setText(polyToString(field.irreducible));

    ui->editButton->setEnabled(true);
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
    QString str = QString::number(p.at[p.len - 1]);
    for (size_t i = p.len - 1; i--; )
        str += " " + QString::number(p.at[i]);
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

void MainWindow::on_updateButton_clicked()
{
    if (editMode) {
        bool ok;
        GfPoly p{};
        const auto arr = ui->irreducibleLine->text().split(" ", Qt::SkipEmptyParts);
        gf_poly_setlen(&p, arr.length());

        for (auto i = arr.length(); i--; ) {
            p.at[i] = arr[arr.length() - i - 1].toUInt(&ok);
            if  (!ok) {
                gf_poly_free(&p);
                QMessageBox::warning(
                            this,
                            "Invalid input",
                            "Polynomial couldn't be parsed, "
                            "expected space seperated multiplicands");
                return;
            }
        }

        updateField(&p);
        gf_poly_free(&p);
    } else {
        updateField();
    }
}

void MainWindow::on_editButton_clicked()
{
    setEditMode(true);
}


void MainWindow::on_calcButton_clicked()
{
    unsigned l, r, res;

    if (!stringToInt(ui->calcLhs->text(), &l))
        return;
    if (!stringToInt(ui->calcRhs->text(), &r))
        return;

    switch (ui->opCombo->currentText()[0].toLatin1()) {
    case '+': res = gfield_add(&field, l, r); break;
    case '-': res = gfield_sub(&field, l, r); break;
    case '*': res = gfield_mul(&field, l, r); break;
    case '/': res = gfield_div(&field, l, r); break;
    default: return;
    }
    ui->calcOutput->setText(QString::number(res));
}
