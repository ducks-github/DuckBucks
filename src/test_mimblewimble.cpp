#include "mimblewimble.h"
#include "mimblewimble_wallet.h"
#include "main.h"
#include "util.h"
#include <iostream>

// This file is for testing/demonstration purposes only
// It shows how the Mimblewimble implementation can be used

void TestMimblewimble(CWallet* pWallet) {
    LogPrintf("Starting Mimblewimble test...\n");
    
    // Initialize the MW wallet
    mw::MimblewimbleWallet mwWallet(pWallet);
    
    // Create some test outputs with different values
    auto [output1, blind1] = mwWallet.CreateOutput(10 * COIN);
    auto [output2, blind2] = mwWallet.CreateOutput(20 * COIN);
    
    // Save these outputs to our wallet (as if we received them)
    mwWallet.SaveOwnedOutput(output1, blind1, 10 * COIN);
    mwWallet.SaveOwnedOutput(output2, blind2, 20 * COIN);
    
    LogPrintf("Created and saved test outputs with total value: %s\n", 
              FormatMoney(30 * COIN).c_str());
    
    // Get our wallet outputs
    std::vector<mw::Output> inputs = mwWallet.GetOwnedOutputs();
    
    // Amounts we want to send
    std::vector<int64_t> amounts = {15 * COIN};
    
    // Create a transaction
    boost::optional<mw::Transaction> txOpt = mwWallet.CreateTransaction(
        inputs,      // Use all our outputs as inputs
        amounts,     // Send 15 DuckBucks
        COIN / 10    // 0.1 DuckBucks fee
    );
    
    if (!txOpt) {
        LogPrintf("Failed to create transaction\n");
        return;
    }
    
    mw::Transaction tx = *txOpt;
    
    // Verify the transaction
    bool isValid = mwWallet.VerifyTransaction(tx);
    LogPrintf("Transaction verification result: %s\n", isValid ? "Valid" : "Invalid");
    
    if (isValid) {
        // In a real implementation, we would:
        // 1. Add the transaction to the mempool
        // 2. Relay it to peers
        // 3. Eventually it would be mined into a block
        
        LogPrintf("Transaction created successfully with hash: %s\n", 
                tx.GetHash().ToString().c_str());
                
        LogPrintf("Number of inputs: %zu\n", tx.vin.size());
        LogPrintf("Number of outputs: %zu\n", tx.vout.size());
        LogPrintf("Number of kernels: %zu\n", tx.kernels.size());
    }
    
    LogPrintf("Mimblewimble test completed\n");
}

// Function to help integrate with the main codebase
void RunMimblewimbleTest(CWallet* pWallet) {
    try {
        TestMimblewimble(pWallet);
    } catch (const std::exception& e) {
        LogPrintf("Mimblewimble test exception: %s\n", e.what());
    }
} 