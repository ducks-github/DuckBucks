#ifndef MIMBLEWIMBLE_WALLET_H
#define MIMBLEWIMBLE_WALLET_H

#include "mimblewimble.h"
#include "wallet.h"
#include <boost/optional.hpp>

namespace mw {

class MimblewimbleWallet {
public:
    MimblewimbleWallet(CWallet* wallet);
    
    /**
     * Creates a confidential output representing a specific amount of DuckBucks
     * @param amount The amount to commit to
     * @return A tuple containing the output and the blinding factor used
     */
    std::pair<Output, BlindingFactor> CreateOutput(int64_t amount);
    
    /**
     * Creates a Mimblewimble transaction using the specified inputs and creating outputs of specified amounts
     * @param inputs The wallet outputs to spend
     * @param amounts The amounts to send
     * @param fee The transaction fee
     * @return An optional transaction object if successful
     */
    boost::optional<Transaction> CreateTransaction(
        const std::vector<Output>& inputs,
        const std::vector<int64_t>& amounts,
        int64_t fee);
    
    /**
     * Sign a transaction with the necessary blinding factors
     * @param tx The transaction to sign
     * @param blindingFactors The blinding factors used for outputs
     * @return Whether the signing was successful
     */
    bool SignTransaction(Transaction& tx, const std::vector<BlindingFactor>& blindingFactors);
    
    /**
     * Verify a received transaction
     * @param tx The transaction to verify
     * @return Whether the transaction is valid
     */
    bool VerifyTransaction(const Transaction& tx) const;
    
    /**
     * Save an output as owned by this wallet
     * @param output The output to save
     * @param blindingFactor The blinding factor for this output
     * @param amount The amount this output represents
     * @return Whether the operation was successful
     */
    bool SaveOwnedOutput(const Output& output, const BlindingFactor& blindingFactor, int64_t amount);
    
    /**
     * Gets all outputs owned by this wallet
     * @return A vector of owned outputs
     */
    std::vector<Output> GetOwnedOutputs() const;
    
    /**
     * Gets the value of a specific output if owned by this wallet
     * @param output The output to check
     * @return The value of the output if owned, or boost::none
     */
    boost::optional<int64_t> GetOutputValue(const Output& output) const;
    
private:
    CWallet* pWallet;
    
    // Map of output commitments to their values and blinding factors
    std::map<Commitment, std::pair<int64_t, BlindingFactor>> ownedOutputs;
};

} // namespace mw

#endif // MIMBLEWIMBLE_WALLET_H 