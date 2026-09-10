#include "crypto.hpp"

#include <sodium.h>

#include <stdexcept>

namespace crypto {

void initialize()
{
    if (sodium_init() < 0) {
        throw std::runtime_error(
            "Failed to initialize libsodium"
        );
    }
}

std::vector<std::uint8_t> deriveKey(
    const std::string& password,
    const std::vector<std::uint8_t>& salt
)
{
    if (salt.size() != crypto_pwhash_SALTBYTES) {
        throw std::runtime_error(
            "Invalid salt size"
        );
    }

    std::vector<std::uint8_t> key(KEY_BYTES);

    const int result = crypto_pwhash(
        key.data(),
        key.size(),
        password.data(),
        password.size(),
        salt.data(),
        crypto_pwhash_OPSLIMIT_MODERATE,
        crypto_pwhash_MEMLIMIT_MODERATE,
        crypto_pwhash_ALG_ARGON2ID13
    );

    if (result != 0) {
        throw std::runtime_error(
            "Key derivation failed"
        );
    }

    return key;
}

std::vector<std::uint8_t> encrypt(
    const std::vector<std::uint8_t>& plaintext,
    const std::vector<std::uint8_t>& key,
    std::vector<std::uint8_t>& nonce
)
{
    if (key.size() != crypto_secretbox_KEYBYTES) {
        throw std::runtime_error(
            "Invalid encryption key size"
        );
    }

    nonce.resize(crypto_secretbox_NONCEBYTES);

    randombytes_buf(
        nonce.data(),
        nonce.size()
    );

    std::vector<std::uint8_t> ciphertext(
        plaintext.size() + crypto_secretbox_MACBYTES
    );

    crypto_secretbox_easy(
        ciphertext.data(),
        plaintext.data(),
        plaintext.size(),
        nonce.data(),
        key.data()
    );

    return ciphertext;
}

std::vector<std::uint8_t> decrypt(
    const std::vector<std::uint8_t>& ciphertext,
    const std::vector<std::uint8_t>& key,
    const std::vector<std::uint8_t>& nonce
)
{
    if (key.size() != crypto_secretbox_KEYBYTES) {
        throw std::runtime_error(
            "Invalid encryption key size"
        );
    }

    if (nonce.size() != crypto_secretbox_NONCEBYTES) {
        throw std::runtime_error(
            "Invalid nonce size"
        );
    }

    if (ciphertext.size() < crypto_secretbox_MACBYTES) {
        throw std::runtime_error(
            "Ciphertext too small"
        );
    }

    std::vector<std::uint8_t> plaintext(
        ciphertext.size() - crypto_secretbox_MACBYTES
    );

    const int result = crypto_secretbox_open_easy(
        plaintext.data(),
        ciphertext.data(),
        ciphertext.size(),
        nonce.data(),
        key.data()
    );

    if (result != 0) {
        throw std::runtime_error(
            "Decryption failed: incorrect password or corrupted vault"
        );
    }

    return plaintext;
}

}