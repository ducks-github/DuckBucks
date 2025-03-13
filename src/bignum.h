#ifndef DUCKBUCKS_BIGNUM_H
#define DUCKBUCKS_BIGNUM_H

#include <openssl/bn.h>
#include <vector>
#include <string>
#include <stdexcept>

// Helper class to manage BN_CTX with RAII
class CAutoBN_CTX {
private:
    BN_CTX* pctx;

public:
    CAutoBN_CTX() {
        pctx = BN_CTX_new();
        if (!pctx) {
            throw std::runtime_error("CAutoBN_CTX: BN_CTX_new failed");
        }
        BN_CTX_start(pctx);
    }

    ~CAutoBN_CTX() {
        if (pctx) {
            BN_CTX_end(pctx);
            BN_CTX_free(pctx);
        }
    }

    BN_CTX* get() const { return pctx; }
    operator BN_CTX*() { return pctx; }
};

// CBigNum class (adapted from Bitcoin's original implementation)
class CBigNum {
private:
    BIGNUM* bn;

public:
    CBigNum() {
        bn = BN_new();
        if (!bn) {
            throw std::runtime_error("CBigNum: BN_new failed");
        }
        BN_zero(bn);
    }

    CBigNum(const CBigNum& b) {
        bn = BN_new();
        if (!bn || !BN_copy(bn, b.bn)) {
            throw std::runtime_error("CBigNum: BN_copy failed");
        }
    }

    CBigNum& operator=(const CBigNum& b) {
        if (!BN_copy(bn, b.bn)) {
            throw std::runtime_error("CBigNum: BN_copy failed");
        }
        return *this;
    }

    ~CBigNum() {
        if (bn) {
            BN_free(bn);
        }
    }

    // Set from integer
    void setuint(unsigned int n) { BN_set_word(bn, n); }
    void setint64(int64_t n) {
        if (n >= 0) {
            BN_set_word(bn, n);
        } else {
            BN_set_word(bn, -n);
            BN_set_negative(bn, 1);
        }
    }

    // Get as integer
    int getint() const {
        if (BN_is_negative(bn)) {
            return -static_cast<int>(BN_get_word(bn));
        }
        return static_cast<int>(BN_get_word(bn));
    }

    // Get as vector of bytes
    std::vector<unsigned char> getvch() const {
        std::vector<unsigned char> vch;
        int size = BN_num_bytes(bn);
        vch.resize(size);
        BN_bn2bin(bn, vch.data());
        // Remove leading zeros
        while (vch.size() > 1 && vch[0] == 0) {
            vch.erase(vch.begin());
        }
        return vch;
    }

    // Set from vector of bytes
    void setvch(const std::vector<unsigned char>& vch) {
        BN_bin2bn(vch.data(), vch.size(), bn);
    }

    // Comparison operators
    bool operator==(const CBigNum& b) const { return BN_cmp(bn, b.bn) == 0; }
    bool operator!=(const CBigNum& b) const { return BN_cmp(bn, b.bn) != 0; }
    bool operator<=(const CBigNum& b) const { return BN_cmp(bn, b.bn) <= 0; }
    bool operator>=(const CBigNum& b) const { return BN_cmp(bn, b.bn) >= 0; }
    bool operator<(const CBigNum& b) const { return BN_cmp(bn, b.bn) < 0; }
    bool operator>(const CBigNum& b) const { return BN_cmp(bn, b.bn) > 0; }

    // Arithmetic (basic support)
    CBigNum operator+(const CBigNum& b) const {
        CBigNum ret;
        if (!BN_add(ret.bn, bn, b.bn)) {
            throw std::runtime_error("CBigNum: BN_add failed");
        }
        return ret;
    }

    CBigNum operator-(const CBigNum& b) const {
        CBigNum ret;
        if (!BN_sub(ret.bn, bn, b.bn)) {
            throw std::runtime_error("CBigNum: BN_sub failed");
        }
        return ret;
    }

    // Additional methods for Bitcoin compatibility
    void sethex(const std::string& str) {
        BN_hex2bn(&bn, str.c_str());
    }

    std::string ToString(int nBase = 10) const {
        char* str = BN_bn2dec(bn);
        std::string result(str);
        OPENSSL_free(str);
        return result;
    }
};

#endif // DUCKBUCKS_BIGNUM_H
