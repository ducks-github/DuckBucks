#include "mimblewimbledialog.h"
#include "ui_mimblewimbledialog.h"

#include "walletmodel.h"
#include "bitcoinunits.h"
#include "guiutil.h"
#include "optionsmodel.h"
#include "main.h"
#include "wallet.h"
#include "init.h"
#include "base58.h"

#include <QMessageBox>
#include <QClipboard>
#include <QVector>
#include <QSortFilterProxyModel>
#include <QTableWidgetItem>

MimblewimbleDialog::MimblewimbleDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MimblewimbleDialog),
    model(0),
    mwWallet(nullptr)
{
    ui->setupUi(this);

    // Set up the output table headers
    ui->outputTable->setColumnCount(3);
    QStringList headers;
    headers << tr("Commitment") << tr("Amount") << tr("Status");
    ui->outputTable->setHorizontalHeaderLabels(headers);
    ui->outputTable->horizontalHeader()->setStretchLastSection(true);

    // Setup transaction input fields
    ui->amountEdit->setValidator(new QDoubleValidator(0, 1000000, 8, this));

    // Initialize with disabled state until we have model
    setEnabled(false);
}

MimblewimbleDialog::~MimblewimbleDialog()
{
    delete ui;
}

void MimblewimbleDialog::setModel(WalletModel *model)
{
    this->model = model;
    
    if (model) {
        // Create MW wallet instance
        mwWallet.reset(new mw::MimblewimbleWallet(model->getWallet()));
        
        // Update UI with current state
        updateUI();
        
        // Enable dialog
        setEnabled(true);
    }
}

void MimblewimbleDialog::updateUI()
{
    if (!model || !mwWallet)
        return;
    
    ui->outputTable->setRowCount(0);
    
    // Get outputs owned by this wallet
    std::vector<mw::Output> outputs = mwWallet->GetOwnedOutputs();
    
    int row = 0;
    int64_t totalAmount = 0;
    
    for (const mw::Output& output : outputs) {
        boost::optional<int64_t> value = mwWallet->GetOutputValue(output);
        if (!value)
            continue;
        
        totalAmount += *value;
        
        ui->outputTable->insertRow(row);
        
        // Commitment hash (abbreviated)
        QString commitmentText = QString::fromStdString(output.commitment.GetHash().ToString().substr(0, 16) + "...");
        QTableWidgetItem *commitmentItem = new QTableWidgetItem(commitmentText);
        commitmentItem->setData(Qt::UserRole, QVariant::fromValue(row)); // Store row index
        ui->outputTable->setItem(row, 0, commitmentItem);
        
        // Amount
        QString amountText = BitcoinUnits::formatWithUnit(
            model->getOptionsModel()->getDisplayUnit(), *value);
        QTableWidgetItem *amountItem = new QTableWidgetItem(amountText);
        ui->outputTable->setItem(row, 1, amountItem);
        
        // Status (always "Unspent" for now)
        QTableWidgetItem *statusItem = new QTableWidgetItem("Unspent");
        ui->outputTable->setItem(row, 2, statusItem);
        
        row++;
    }
    
    // Update total amount
    QString totalText = BitcoinUnits::formatWithUnit(
        model->getOptionsModel()->getDisplayUnit(), totalAmount);
    ui->totalLabel->setText(tr("Total Available: %1").arg(totalText));
}

void MimblewimbleDialog::on_createButton_clicked()
{
    if (!model || !mwWallet)
        return;
    
    // Get amount
    bool ok = false;
    double amount = ui->amountEdit->text().toDouble(&ok);
    if (!ok || amount <= 0) {
        QMessageBox::warning(this, tr("Invalid Amount"),
            tr("Please enter a valid amount to send."),
            QMessageBox::Ok, QMessageBox::Ok);
        return;
    }
    
    // Convert amount to satoshis
    int unit = model->getOptionsModel()->getDisplayUnit();
    int64_t nAmount = BitcoinUnits::toCoinUnits(unit, amount);
    
    // Check if we have enough outputs
    std::vector<mw::Output> inputs = mwWallet->GetOwnedOutputs();
    if (inputs.empty()) {
        QMessageBox::warning(this, tr("No Inputs"),
            tr("You don't have any Mimblewimble outputs to spend."),
            QMessageBox::Ok, QMessageBox::Ok);
        return;
    }
    
    // Ask for confirmation
    QString question = tr("Are you sure you want to create a Mimblewimble transaction "
                         "sending %1 to a blinded address?")
                        .arg(BitcoinUnits::formatWithUnit(unit, nAmount));
    
    QMessageBox::StandardButton retval = showConfirmationDialog(
        tr("Confirm Send"), question);
    
    if (retval != QMessageBox::Yes)
        return;
    
    // Create transaction
    std::vector<int64_t> amounts = {nAmount};
    int64_t fee = COIN / 100; // 0.01 coin fee
    
    boost::optional<mw::Transaction> txOpt = mwWallet->CreateTransaction(
        inputs, amounts, fee);
    
    if (!txOpt) {
        QMessageBox::critical(this, tr("Transaction Failed"),
            tr("Failed to create transaction. Please check your inputs and try again."),
            QMessageBox::Ok, QMessageBox::Ok);
        return;
    }
    
    mw::Transaction tx = *txOpt;
    
    // Show success message
    QMessageBox::information(this, tr("Transaction Created"),
        tr("Transaction created successfully with hash: %1")
            .arg(QString::fromStdString(tx.GetHash().ToString())),
        QMessageBox::Ok, QMessageBox::Ok);
    
    // Clear amount field
    ui->amountEdit->clear();
    
    // Update UI
    updateUI();
}

void MimblewimbleDialog::on_refreshButton_clicked()
{
    updateUI();
}

void MimblewimbleDialog::on_testButton_clicked()
{
    if (!model || !mwWallet)
        return;
    
    // Generate a test output for 5 coins
    auto [output, blind] = mwWallet->CreateOutput(5 * COIN);
    
    // Save to wallet
    if (mwWallet->SaveOwnedOutput(output, blind, 5 * COIN)) {
        QMessageBox::information(this, tr("Test Output Created"),
            tr("A test output of 5 coins has been created for demonstration purposes."),
            QMessageBox::Ok, QMessageBox::Ok);
        
        updateUI();
    } else {
        QMessageBox::warning(this, tr("Creation Failed"),
            tr("Failed to create test output."),
            QMessageBox::Ok, QMessageBox::Ok);
    }
}

QMessageBox::StandardButton MimblewimbleDialog::showConfirmationDialog(const QString& title, const QString& text)
{
    return QMessageBox::question(this, title, text,
        QMessageBox::Yes|QMessageBox::Cancel, QMessageBox::Cancel);
} 