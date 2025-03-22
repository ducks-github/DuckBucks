#ifndef MIMBLEWIMBLEDIALOG_H
#define MIMBLEWIMBLEDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include "walletmodel.h"
#include "mimblewimble_wallet.h"

namespace Ui {
    class MimblewimbleDialog;
}

/** Dialog for creating and viewing Mimblewimble transactions */
class MimblewimbleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MimblewimbleDialog(QWidget *parent = 0);
    ~MimblewimbleDialog();

    void setModel(WalletModel *model);

private:
    Ui::MimblewimbleDialog *ui;
    WalletModel *model;
    std::unique_ptr<mw::MimblewimbleWallet> mwWallet;
    QMessageBox::StandardButton showConfirmationDialog(const QString& title, const QString& text);

private slots:
    /** Create a new Mimblewimble transaction */
    void on_createButton_clicked();
    
    /** Refresh the list of owned Mimblewimble outputs */
    void on_refreshButton_clicked();
    
    /** Create a test output for demonstration purposes */
    void on_testButton_clicked();
    
    /** Update the UI based on current wallet state */
    void updateUI();
};

#endif // MIMBLEWIMBLEDIALOG_H 