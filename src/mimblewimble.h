#ifndef MIMBLEWIMBLE_H
#define MIMBLEWIMBLE_H

#include "serialize.h"
#include "uint256.h"
#include <vector>

namespace mw {

// Basic implementation of Mimblewimble protocol

class BlindingFactor {
private:
    std::vector<unsigned char> vch;

public:
    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(vch);
    }

    BlindingFactor();
    BlindingFactor(const std::vector<unsigned char>& bytes) : vch(bytes) {}

    const std::vector<unsigned char>& GetBytes() const;
    static BlindingFactor Random();
    void operator+=(const BlindingFactor& rhs);
};

// Pedersen Commitment (v*H + r*G)
class Commitment {
private:
    std::vector<unsigned char> vch;

public:
    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(vch);
    }

    Commitment() {}
    Commitment(const std::vector<unsigned char>& bytes) : vch(bytes) {}

    bool IsValid() const;
    uint256 GetHash() const;
};

// Zero-knowledge range proof
class RangeProof {
private:
    std::vector<unsigned char> vch;

public:
    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(vch);
    }

    RangeProof() {}
    RangeProof(const std::vector<unsigned char>& bytes) : vch(bytes) {}
    
    bool Verify(const Commitment& commitment) const;
};

// Mimblewimble output (UTXO)
class Output {
public:
    Commitment commitment;
    RangeProof rangeProof;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(commitment);
        READWRITE(rangeProof);
    }

    Output() {}
    Output(const Commitment& commit, const RangeProof& proof) 
        : commitment(commit), rangeProof(proof) {}
    
    bool Verify() const;
    uint256 GetHash() const;
};

// Mimblewimble input
class Input {
public:
    Commitment commitment;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(commitment);
    }

    Input() {}
    Input(const Commitment& commit) : commitment(commit) {}
    
    uint256 GetHash() const;
};

// Transaction kernel
class Kernel {
public:
    int64 nFee;
    unsigned int nLockHeight;
    Commitment excess;
    std::vector<unsigned char> signature;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(nFee);
        READWRITE(nLockHeight);
        READWRITE(excess);
        READWRITE(signature);
    }

    Kernel() : nFee(0), nLockHeight(0) {}
    
    uint256 GetHash() const;
    bool Verify() const;
};

// Complete Mimblewimble transaction
class Transaction {
public:
    std::vector<Input> vin;
    std::vector<Output> vout;
    std::vector<Kernel> kernels;
    BlindingFactor offset;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(vin);
        READWRITE(vout);
        READWRITE(kernels);
        READWRITE(offset);
    }

    Transaction() {}
    
    uint256 GetHash() const;
    bool Verify() const;
    
    static Transaction BuildTransaction(
        const std::vector<Output>& inputs,
        const std::vector<Output>& outputs,
        int64 fee,
        unsigned int lockHeight);
};

// Helper functions
Commitment createCommitment(uint64_t value, const BlindingFactor& blindingFactor);
RangeProof createRangeProof(uint64_t value, const BlindingFactor& blindingFactor);
Output createOutput(uint64_t value, const BlindingFactor& blindingFactor);

} // namespace mw

#endif // MIMBLEWIMBLE_H
