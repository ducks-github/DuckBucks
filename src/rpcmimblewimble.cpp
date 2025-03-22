#include "mimblewimble.h"
#include "mimblewimble_wallet.h"
#include "mimblewimble_init.h"
#include "init.h"
#include "bitcoinrpc.h"
#include "base58.h"

using namespace std;
using namespace json_spirit;

// Forward declaration
extern std::unique_ptr<mw::MimblewimbleWallet> g_pMWWallet;

Value runmwtest(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "runmwtest\n"
            "Run Mimblewimble test transaction.\n");

    if (!g_pMWWallet)
        throw runtime_error("Mimblewimble wallet not initialized");

    RunMimblewimbleTest();
    
    return Value::null;
}

Value getmwbalance(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 0)
        throw runtime_error(
            "getmwbalance\n"
            "Returns the wallet's Mimblewimble private balance.\n");

    if (!g_pMWWallet)
        throw runtime_error("Mimblewimble wallet not initialized");
    
    // Get all outputs owned by the wallet
    std::vector<mw::Output> outputs = g_pMWWallet->GetOwnedOutputs();
    
    int64_t balance = 0;
    for (const mw::Output& output : outputs) {
        boost::optional<int64_t> value = g_pMWWallet->GetOutputValue(output);
        if (value) {
            balance += *value;
        }
    }
    
    return ValueFromAmount(balance);
}

Value createmwoutput(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "createmwoutput <amount>\n"
            "Create a test Mimblewimble output for demonstration purposes.\n"
            "This is only for testing and should not be used in production.\n"
            "<amount> is the amount in " + CURRENCY_UNIT + " to create\n");

    if (!g_pMWWallet)
        throw runtime_error("Mimblewimble wallet not initialized");
    
    int64_t amount = AmountFromValue(params[0]);
    
    auto [output, blind] = g_pMWWallet->CreateOutput(amount);
    bool success = g_pMWWallet->SaveOwnedOutput(output, blind, amount);
    
    if (!success)
        throw runtime_error("Failed to save output to wallet");
    
    Object result;
    result.push_back(Pair("commitment", output.commitment.GetHash().ToString()));
    result.push_back(Pair("amount", ValueFromAmount(amount)));
    
    return result;
}

Value createmwtransaction(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "createmwtransaction <amount>\n"
            "Create a Mimblewimble transaction sending the specified amount.\n"
            "<amount> is the amount in " + CURRENCY_UNIT + " to send\n");

    if (!g_pMWWallet)
        throw runtime_error("Mimblewimble wallet not initialized");
    
    int64_t amount = AmountFromValue(params[0]);
    
    // Get all outputs owned by the wallet
    std::vector<mw::Output> inputs = g_pMWWallet->GetOwnedOutputs();
    
    if (inputs.empty())
        throw runtime_error("No Mimblewimble outputs available to spend");
    
    // Create the transaction
    std::vector<int64_t> amounts = {amount};
    int64_t fee = COIN / 100; // 0.01 coin fee
    
    boost::optional<mw::Transaction> txOpt = g_pMWWallet->CreateTransaction(
        inputs, amounts, fee);
    
    if (!txOpt)
        throw runtime_error("Failed to create transaction");
    
    mw::Transaction tx = *txOpt;
    
    Object result;
    result.push_back(Pair("txid", tx.GetHash().ToString()));
    result.push_back(Pair("fee", ValueFromAmount(fee)));
    result.push_back(Pair("inputs", (int)tx.vin.size()));
    result.push_back(Pair("outputs", (int)tx.vout.size()));
    
    return result;
}

// Add RPC command table entries
static const CRPCCommand commands[] =
{ //  category              name                      actor (function)         okSafeMode
  //  --------------------- ------------------------  -----------------------  ----------
    { "mimblewimble",       "runmwtest",              &runmwtest,              true,   },
    { "mimblewimble",       "getmwbalance",           &getmwbalance,           true,   },
    { "mimblewimble",       "createmwoutput",         &createmwoutput,         true,   },
    { "mimblewimble",       "createmwtransaction",    &createmwtransaction,    true,   },
}; 