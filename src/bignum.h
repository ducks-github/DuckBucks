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
    BIGNUM bn;  // Composition instead of inheritance

public:
    // Constructor - initialize the BIGNUM structure
    CBigNum() {
        BN_init(&bn);
    }

    // Copy constructor - deep copy the BIGNUM
    CBigNum(const CBigNum& b) {
        BN_init(&bn);
        if (!BN_copy(&bn, &b.bn))
            throw std::runtime_error("CBigNum::CBigNum(const CBigNum&) : BN_copy failed");
    }

    // Destructor - properly free BIGNUM resources
    ~CBigNum() {
        BN_clear_free(&bn);
    }

    // Assignment operator
    CBigNum& operator=(const CBigNum& b) {
        if (!BN_copy(&bn, &b.bn))
            throw std::runtime_error("CBigNum::operator= : BN_copy failed");
        return (*this);
    }

    // Integer initialization
    void setuint(unsigned int n) {
        BN_set_word(&bn, n);
    }

    void setint64(int64_t n) {
        if (n >= 0) {
            BN_set_word(&bn, n);
        } else {
            BN_set_word(&bn, -n);
            BN_set_negative(&bn, 1);
        }
    }

    // Comparison operators
    bool operator==(const CBigNum& b) const { return BN_cmp(&bn, &b.bn) == 0; }
    bool operator!=(const CBigNum& b) const { return BN_cmp(&bn, &b.bn) != 0; }
    bool operator<=(const CBigNum& b) const { return BN_cmp(&bn, &b.bn) <= 0; }
    bool operator>=(const CBigNum& b) const { return BN_cmp(&bn, &b.bn) >= 0; }
    bool operator<(const CBigNum& b) const { return BN_cmp(&bn, &b.bn) < 0; }
    bool operator>(const CBigNum& b) const { return BN_cmp(&bn, &b.bn) > 0; }

    // Arithmetic operators
    CBigNum operator+(const CBigNum& b) const {
        CBigNum ret;
        if (!BN_add(&ret.bn, &bn, &b.bn))
            throw std::runtime_error("CBigNum::operator+ : BN_add failed");
        return ret;
    }

    CBigNum operator-(const CBigNum& b) const {
        CBigNum ret;
        if (!BN_sub(&ret.bn, &bn, &b.bn))
            throw std::runtime_error("CBigNum::operator- : BN_sub failed");
        return ret;
    }

    // Conversion methods
    std::vector<unsigned char> getvch() const {
        std::vector<unsigned char> vch;
        int size = BN_num_bytes(&bn);
        vch.resize(size);
        BN_bn2bin(&bn, vch.data());
        while (vch.size() > 1 && vch[0] == 0)
            vch.erase(vch.begin());
        return vch;
    }

    void setvch(const std::vector<unsigned char>& vch) {
        BN_bin2bn(vch.data(), vch.size(), &bn);
    }

    std::string ToString(int nBase = 10) const {
        char* str = BN_bn2dec(&bn);
        std::string result(str);
        OPENSSL_free(str);
        return result;
    }
};

#endif // DUCKBUCKS_BIGNUM_H
