#include "mimblewimble_wallet.h"
#include "util.h"
#include <openssl/rand.h>

namespace mw {

MimblewimbleWallet::MimblewimbleWallet(CWallet* wallet) : pWallet(wallet) {}

std::pair<Output, BlindingFactor> MimblewimbleWallet::CreateOutput(int64_t amount) {
    // Generate a random blinding factor
    BlindingFactor blind = BlindingFactor::Random();
    
    // Create commitment using the amount and blinding factor
    Commitment commitment = createCommitment(amount, blind);
    
    // Create a range proof that proves amount is positive without revealing it
    RangeProof rangeProof = createRangeProof(amount, blind);
    
    // Create and return the output
    Output output(commitment, rangeProof);
    return std::make_pair(output, blind);
}

boost::optional<Transaction> MimblewimbleWallet::CreateTransaction(
    const std::vector<Output>& inputs,
    const std::vector<int64_t>& amounts,
    int64_t fee)
{
    // Calculate total input value
    int64_t inputTotal = 0;
    std::vector<BlindingFactor> inputBlindingFactors;
    
    for (const Output& input : inputs) {
        boost::optional<int64_t> value = GetOutputValue(input);
        if (!value) {
            LogPrintf("MW: Cannot spend output not owned by wallet\n");
            return boost::none;
        }
        
        inputTotal += *value;
        
        // Find blinding factor for this input
        auto it = ownedOutputs.find(input.commitment);
        if (it != ownedOutputs.end()) {
            inputBlindingFactors.push_back(it->second.second);
        } else {
            LogPrintf("MW: Missing blinding factor for input\n");
            return boost::none;
        }
    }
    
    // Calculate total output value
    int64_t outputTotal = 0;
    for (int64_t amount : amounts) {
        outputTotal += amount;
    }
    
    // Ensure we have enough funds (including fee)
    if (inputTotal < outputTotal + fee) {
        LogPrintf("MW: Insufficient funds: %d < %d + %d\n", inputTotal, outputTotal, fee);
        return boost::none;
    }
    
    // Handle change if necessary
    int64_t change = inputTotal - outputTotal - fee;
    if (change < 0) {
        LogPrintf("MW: Invalid change amount\n");
        return boost::none;
    }
    
    // Create outputs
    std::vector<Output> outputs;
    std::vector<BlindingFactor> outputBlindingFactors;
    
    for (int64_t amount : amounts) {
        auto [output, blind] = CreateOutput(amount);
        outputs.push_back(output);
        outputBlindingFactors.push_back(blind);
    }
    
    // Add change output if needed
    if (change > 0) {
        auto [changeOutput, changeBlind] = CreateOutput(change);
        outputs.push_back(changeOutput);
        outputBlindingFactors.push_back(changeBlind);
    }
    
    // Build the transaction
    Transaction tx = Transaction::BuildTransaction(
        inputs,
        outputs,
        fee,
        pWallet->GetChainTip()->nHeight // Current lock height
    );
    
    // Sign the transaction
    if (!SignTransaction(tx, outputBlindingFactors)) {
        LogPrintf("MW: Failed to sign transaction\n");
        return boost::none;
    }
    
    // Save our outputs to wallet
    size_t outputIdx = 0;
    for (int64_t amount : amounts) {
        if (!SaveOwnedOutput(outputs[outputIdx], outputBlindingFactors[outputIdx], amount)) {
            LogPrintf("MW: Failed to save output to wallet\n");
        }
        outputIdx++;
    }
    
    // Save change output
    if (change > 0) {
        if (!SaveOwnedOutput(outputs.back(), outputBlindingFactors.back(), change)) {
            LogPrintf("MW: Failed to save change output to wallet\n");
        }
    }
    
    return tx;
}

bool MimblewimbleWallet::SignTransaction(Transaction& tx, const std::vector<BlindingFactor>& blindingFactors) {
    // In a real implementation, we would:
    // 1. Calculate the transaction offset
    // 2. Calculate the kernel excess from input/output blinding factors and offset
    // 3. Sign the kernel with the excess as private key
    
    // For now, we just create a dummy signature
    if (tx.kernels.empty()) {
        LogPrintf("MW: No kernels to sign\n");
        return false;
    }
    
    // Just set a dummy excess for now - in real implementation this would be calculated
    BlindingFactor excess = BlindingFactor::Random();
    tx.kernels[0].excess = createCommitment(0, excess);
    
    return true;
}

bool MimblewimbleWallet::VerifyTransaction(const Transaction& tx) const {
    return tx.Verify();
}

bool MimblewimbleWallet::SaveOwnedOutput(
    const Output& output,
    const BlindingFactor& blindingFactor,
    int64_t amount)
{
    ownedOutputs[output.commitment] = std::make_pair(amount, blindingFactor);
    return true;
}

std::vector<Output> MimblewimbleWallet::GetOwnedOutputs() const {
    std::vector<Output> outputs;
    for (const auto& [commitment, valueAndBlind] : ownedOutputs) {
        // Note: In a real implementation, we'd reconstruct the output with the
        // correct range proof from stored data
        Output output;
        output.commitment = commitment;
        outputs.push_back(output);
    }
    return outputs;
}

boost::optional<int64_t> MimblewimbleWallet::GetOutputValue(const Output& output) const {
    auto it = ownedOutputs.find(output.commitment);
    if (it != ownedOutputs.end()) {
        return it->second.first;
    }
    return boost::none;
}

} // namespace mw 