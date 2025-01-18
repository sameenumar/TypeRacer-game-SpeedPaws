#ifndef MODESELECTIONDIALOG_H
#define MODESELECTIONDIALOG_H

#include <QDialog>

namespace Ui {
class ModeSelectionDialog;
}

class ModeSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ModeSelectionDialog(QWidget *parent = nullptr);
    ~ModeSelectionDialog();
signals:
    void modeSelected(const QString &mode);


private slots:
    void on_easy_clicked();

    void on_medium_clicked();

    void on_hard_clicked();



private:
    Ui::ModeSelectionDialog *ui;
};

#endif // MODESELECTIONDIALOG_H
