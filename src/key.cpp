// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "key.h"
#include <openssl/ecdsa.h>
#include <openssl/rand.h>
#include <openssl/obj_mac.h>
#include <vector>
#include "util.h"

namespace {

class CECKey {
private:
    EC_KEY *pkey;

public:
    CECKey() {
        pkey = EC_KEY_new_by_curve_name(NID_secp256k1);
        if (!pkey) throw key_error("CECKey::CECKey() : EC_KEY_new_by_curve_name failed");
    }

    ~CECKey() {
        if (pkey) EC_KEY_free(pkey);
    }

    EC_KEY* GetECKey() const { return pkey; }

    void SetCompressedPubKey(bool fCompressed) {
        EC_KEY_set_conv_form(pkey, fCompressed ? POINT_CONVERSION_COMPRESSED : POINT_CONVERSION_UNCOMPRESSED);
    }

    void GetSecretBytes(unsigned char vch[32]) const {
        const BIGNUM *bn = EC_KEY_get0_private_key(pkey);
        int n = BN_bn2bin(bn, vch);
        if (n != 32)
            throw key_error("CECKey::GetSecretBytes : BN_bn2bin failed");
    }

    bool SetPrivKey(const CPrivKey &vchPrivKey, bool fCompressed) {
        if (!EC_KEY_oct2priv(pkey, vchPrivKey.data(), vchPrivKey.size()))
            return false;
        EC_KEY_set_conv_form(pkey, fCompressed ? POINT_CONVERSION_COMPRESSED : POINT_CONVERSION_UNCOMPRESSED);
        return true;
    }

    CPrivKey GetPrivKey() const {
        size_t nSize = i2d_ECPrivateKey(pkey, NULL);
        if (!nSize)
            throw key_error("CECKey::GetPrivKey() : i2d_ECPrivateKey failed");
        CPrivKey vchPrivKey(nSize, 0);
        unsigned char *pbegin = &vchPrivKey[0];
        if (!i2d_ECPrivateKey(pkey, &pbegin))
            throw key_error("CECKey::GetPrivKey() : i2d_ECPrivateKey failed");
        return vchPrivKey;
    }

    bool SetPubKey(const std::vector<unsigned char> &vchPubKey) {
        const unsigned char *pbegin = vchPubKey.data();
        return EC_KEY_oct2key(pkey, pbegin, vchPubKey.size(), NULL) != 0;
    }

    CPubKey GetPubKey() const {
        int nSize = i2o_ECPublicKey(pkey, NULL);
        if (!nSize)
            throw key_error("CECKey::GetPubKey() : i2o_ECPublicKey failed");
        std::vector<unsigned char> vchPubKey(nSize, 0);
        unsigned char *pbegin = &vchPubKey[0];
        if (!i2o_ECPublicKey(pkey, &pbegin))
            throw key_error("CECKey::GetPubKey() : i2o_ECPublicKey failed");
        return CPubKey(vchPubKey);
    }

    void MakeNewKey(bool fCompressed) {
        if (!EC_KEY_generate_key(pkey))
            throw key_error("CECKey::MakeNewKey() : EC_KEY_generate_key failed");
        EC_KEY_set_conv_form(pkey, fCompressed ? POINT_CONVERSION_COMPRESSED : POINT_CONVERSION_UNCOMPRESSED);
    }

    bool Sign(const uint256 &hash, std::vector<unsigned char> &vchSig) const {
        vchSig.clear();
        unsigned char pchSig[10000];
        unsigned int nSize = 0;
        if (!ECDSA_sign(0, hash.begin(), hash.size(), pchSig, &nSize, pkey))
            return false;
        vchSig.assign(pchSig, pchSig + nSize);
        return true;
    }

    bool Verify(const uint256 &hash, const std::vector<unsigned char> &vchSig) const {
        return ECDSA_verify(0, hash.begin(), hash.size(), vchSig.data(), vchSig.size(), pkey) == 1;
    }

    bool SignCompact(const uint256 &hash, std::vector<unsigned char> &vchSig) const;
};

// Standalone function for ECDSA recovery
int ECDSA_SIG_recover_key_GFp(EC_KEY *eckey, ECDSA_SIG *sig, const unsigned char *msg, int msglen, int recid, int check) {
    if (!eckey) return 0;

    int ret = 0;
    BN_CTX *ctx = BN_CTX_new();
    if (!ctx) return -1;

    const BIGNUM *r = nullptr;
    const BIGNUM *s = nullptr;
    ECDSA_SIG_get0(sig, &r, &s);

    BIGNUM *order = BN_new();
    BIGNUM *x = BN_new();
    BIGNUM *e = BN_new();
    BIGNUM *field = BN_new();
    EC_POINT *R = nullptr;
    EC_POINT *O = nullptr;
    EC_POINT *Q = nullptr;
    const EC_GROUP *group = EC_KEY_get0_group(eckey);

    if (!order || !x || !e || !field) { ret = -1; goto err; }
    if (!EC_GROUP_get_order(group, order, ctx)) { ret = -2; goto err; }
    if (!BN_set_word(x, recid / 2)) { ret = -1; goto err; }
    if (!BN_mul_word(x, 2)) { ret = -1; goto err; }
    if (!BN_add(x, x, r)) { ret = -1; goto err; }
    if (!EC_GROUP_get_curve_GFp(group, field, NULL, NULL, ctx)) { ret = -2; goto err; }
    if (BN_cmp(x, field) >= 0) { ret = 0; goto err; }

    R = EC_POINT_new(group);
    if (!R) { ret = -2; goto err; }
    if (!EC_POINT_set_compressed_coordinates_GFp(group, R, x, recid % 2, ctx)) { ret = 0; goto err; }

    O = EC_POINT_new(group);
    if (!O) { ret = -2; goto err; }
    if (!EC_POINT_mul(group, O, NULL, R, order, ctx)) { ret = -2; goto err; }

    Q = EC_POINT_new(group);
    if (!Q) { ret = -2; goto err; }

    BN_set_negative(e, 1);
    if (!BN_mod_add(e, order, e, order, ctx)) { ret = -1; goto err; }
    if (!EC_POINT_mul(group, Q, e, O, s, ctx)) { ret = -2; goto err; }

    if (check) {
        if (!EC_KEY_set_public_key(eckey, Q)) { ret = -2; goto err; }
        ret = (ECDSA_do_verify(msg, msglen, sig, eckey) == 1) ? 1 : 0;
    } else {
        if (!EC_KEY_set_public_key(eckey, Q)) { ret = -2; goto err; }
        ret = 1;
    }

err:
    if (ctx) BN_CTX_free(ctx);
    if (order) BN_free(order);
    if (x) BN_free(x);
    if (e) BN_free(e);
    if (field) BN_free(field);
    if (R) EC_POINT_free(R);
    if (O) EC_POINT_free(O);
    if (Q) EC_POINT_free(Q);
    return ret;
}

bool CECKey::SignCompact(const uint256 &hash, std::vector<unsigned char> &vchSig) const {
    vchSig.clear();
    ECDSA_SIG *sig = ECDSA_do_sign(hash.begin(), hash.size(), pkey);
    if (!sig)
        return false;

    vchSig.resize(65);
    int nRecId = -1;
    for (int i = 0; i < 4; i++) {
        CECKey eckeyTemp;
        eckeyTemp.MakeNewKey(false); // Temporary key for recovery
        if (ECDSA_SIG_recover_key_GFp(eckeyTemp.GetECKey(), sig, hash.begin(), hash.size(), i, 0) == 1) {
            if (GetPubKey() == eckeyTemp.GetPubKey()) {
                nRecId = i;
                break;
            }
        }
    }

    if (nRecId == -1) {
        ECDSA_SIG_free(sig);
        return false;
    }

    vchSig[0] = nRecId + 27 + (EC_KEY_get_conv_form(pkey) == POINT_CONVERSION_COMPRESSED ? 4 : 0);
    const BIGNUM *r = nullptr;
    const BIGNUM *s = nullptr;
    ECDSA_SIG_get0(sig, &r, &s);
    BN_bn2binpad(r, &vchSig[1], 32);
    BN_bn2binpad(s, &vchSig[33], 32);

    ECDSA_SIG_free(sig);
    return true;
}

} // anonymous namespace

// CKey implementations
bool CKey::Check(const unsigned char *vch) {
    BIGNUM *bn = BN_new();
    bool ret = false;
    if (bn && BN_bin2bn(vch, 32, bn) && BN_is_zero(bn) == 0) {
        ret = true; // Add more checks if needed
    }
    if (bn) BN_free(bn);
    return ret;
}

void CKey::MakeNewKey(bool fCompressed) {
    CECKey ckey;
    ckey.MakeNewKey(fCompressed);
    unsigned char vch[32];
    ckey.GetSecretBytes(vch);
    Set(vch, vch + 32, fCompressed);
}

bool CKey::SetPrivKey(const CPrivKey &vchPrivKey, bool fCompressed) {
    CECKey ckey;
    if (!ckey.SetPrivKey(vchPrivKey, fCompressed))
        return false;
    unsigned char vch[32];
    ckey.GetSecretBytes(vch);
    Set(vch, vch + 32, fCompressed);
    return true;
}

CPrivKey CKey::GetPrivKey() const {
    if (!fValid)
        throw key_error("CKey::GetPrivKey() : key not valid");
    CECKey ckey;
    ckey.SetPrivKey(CPrivKey(vch, vch + 32), fCompressed);
    return ckey.GetPrivKey();
}

CPubKey CKey::GetPubKey() const {
    if (!fValid)
        throw key_error("CKey::GetPubKey() : key not valid");
    CECKey ckey;
    ckey.SetPrivKey(CPrivKey(vch, vch + 32), fCompressed);
    return ckey.GetPubKey();
}

bool CKey::Sign(const uint256 &hash, std::vector<unsigned char> &vchSig) const {
    if (!fValid)
        throw key_error("CKey::Sign() : key not valid");
    CECKey ckey;
    ckey.SetPrivKey(CPrivKey(vch, vch + 32), fCompressed);
    return ckey.Sign(hash, vchSig);
}

bool CKey::SignCompact(const uint256 &hash, std::vector<unsigned char> &vchSig) const {
    if (!fValid)
        throw key_error("CKey::SignCompact() : key not valid");
    CECKey ckey;
    ckey.SetPrivKey(CPrivKey(vch, vch + 32), fCompressed);
    return ckey.SignCompact(hash, vchSig);
}

// CPubKey implementations
bool CPubKey::IsFullyValid() const {
    if (!IsValid())
        return false;
    CECKey ckey;
    if (!ckey.SetPubKey(std::vector<unsigned char>(vch, vch + size())))
        return false;
    return EC_KEY_check_key(ckey.GetECKey());
}

bool CPubKey::Verify(const uint256 &hash, const std::vector<unsigned char> &vchSig) const {
    if (!IsValid())
        return false;
    CECKey ckey;
    if (!ckey.SetPubKey(std::vector<unsigned char>(vch, vch + size())))
        return false;
    return ckey.Verify(hash, vchSig);
}

bool CPubKey::VerifyCompact(const uint256 &hash, const std::vector<unsigned char> &vchSig) const {
    if (vchSig.size() != 65)
        return false;
    CECKey ckey;
    int nRecId = (vchSig[0] - 27) & 3;
    bool fCompressed = ((vchSig[0] - 27) & 4) != 0;
    ECDSA_SIG *sig = ECDSA_SIG_new();
    if (!sig) return false;

    BIGNUM *r = BN_new();
    BIGNUM *s = BN_new();
    if (!r || !s || !BN_bin2bn(&vchSig[1], 32, r) || !BN_bin2bn(&vchSig[33], 32, s) ||
        !ECDSA_SIG_set0(sig, r, s)) {
        BN_free(r);
        BN_free(s);
        ECDSA_SIG_free(sig);
        return false;
    }

    bool ret = ECDSA_SIG_recover_key_GFp(ckey.GetECKey(), sig, hash.begin(), hash.size(), nRecId, 1) == 1 &&
              ckey.GetPubKey() == *this && fCompressed == IsCompressed();
    ECDSA_SIG_free(sig);
    return ret;
}

bool CPubKey::RecoverCompact(const uint256 &hash, const std::vector<unsigned char> &vchSig) {
    if (vchSig.size() != 65)
        return false;
    CECKey ckey;
    int nRecId = (vchSig[0] - 27) & 3;
    ECDSA_SIG *sig = ECDSA_SIG_new();
    if (!sig) return false;

    BIGNUM *r = BN_new();
    BIGNUM *s = BN_new();
    if (!r || !s || !BN_bin2bn(&vchSig[1], 32, r) || !BN_bin2bn(&vchSig[33], 32, s) ||
        !ECDSA_SIG_set0(sig, r, s)) {
        BN_free(r);
        BN_free(s);
        ECDSA_SIG_free(sig);
        return false;
    }

    if (ECDSA_SIG_recover_key_GFp(ckey.GetECKey(), sig, hash.begin(), hash.size(), nRecId, 0) != 1) {
        ECDSA_SIG_free(sig);
        return false;
    }
    Set(ckey.GetPubKey().begin(), ckey.GetPubKey().end());
    ECDSA_SIG_free(sig);
    return true;
}

bool CPubKey::Decompress() {
    if (!IsCompressed())
        return true;
    CECKey ckey;
    if (!ckey.SetPubKey(std::vector<unsigned char>(vch, vch + size())))
        return false;
    ckey.SetCompressedPubKey(false);
    CPubKey newKey = ckey.GetPubKey();
    Set(newKey.begin(), newKey.end());
    return true;
}
