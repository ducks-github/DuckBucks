#pragma once

#include <array>
#include <vector>
#include <span>

class Crypto {
public:
    // Use modern key derivation
    static std::vector<uint8_t> deriveKey(std::span<const uint8_t> password) {
        // Use Argon2id instead of older algorithms
        return argon2id(password);
    }
    
    // Use modern authenticated encryption
    static std::vector<uint8_t> encrypt(
        std::span<const uint8_t> data,
        std::span<const uint8_t> key
    ) {
        // Use AES-256-GCM instead of older modes
        return aesGcmEncrypt(data, key);
    }
    
    // Use secure random number generation
    static std::array<uint8_t, 32> generateRandomBytes() {
        std::array<uint8_t, 32> bytes;
        if (!RAND_bytes(bytes.data(), bytes.size())) {
            throw std::runtime_error("Failed to generate random bytes");
        }
        return bytes;
    }
    
    // Modern hash functions
    static std::array<uint8_t, 32> hash(std::span<const uint8_t> data) {
        // Use SHA3-256 instead of SHA256
        return sha3_256(data);
    }
};