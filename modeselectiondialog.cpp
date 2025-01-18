#include "modeselectiondialog.h"
#include "ui_modeselectiondialog.h"

ModeSelectionDialog::ModeSelectionDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ModeSelectionDialog)
{
    ui->setupUi(this);

}

ModeSelectionDialog::~ModeSelectionDialog()
{
    delete ui;
}

void ModeSelectionDialog::on_easy_clicked()
{
    emit modeSelected("Easy"); // Emit signal with selected mode
    accept(); // Close the dialog
}


void ModeSelectionDialog::on_medium_clicked()
{
    emit modeSelected("Medium"); // Emit signal with selected mode
    accept(); // Close the dialog
}


void ModeSelectionDialog::on_hard_clicked()
{
    emit modeSelected("Hard"); // Emit signal with selected mode
    accept(); // Close the dialog
}


