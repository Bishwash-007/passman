#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <sodium.h>

#include "crypto.hpp"

int main()
{
    crypto::initialize();

    const std::string password = "test-password";
    const std::string plaintextText =
        "github|bishwash|super-secret-password";
    const std::vector<std::uint8_t> plaintext(
        plaintextText.begin(), plaintextText.end());
    std::vector<std::uint8_t> salt(crypto_pwhash_SALTBYTES);
    randombytes_buf(salt.data(), salt.size());
    std::vector<std::uint8_t> secondSalt(crypto_pwhash_SALTBYTES);
    randombytes_buf(secondSalt.data(), secondSalt.size());
    assert(secondSalt != salt);

    const std::vector<std::uint8_t> key =
        crypto::deriveKey(password, salt);
    assert(crypto::deriveKey(password, salt) == key);
    assert(crypto::deriveKey(password, secondSalt) != key);
    std::vector<std::uint8_t> nonce;
    const std::vector<std::uint8_t> ciphertext =
        crypto::encrypt(plaintext, key, nonce);
    const std::vector<std::uint8_t> decrypted =
        crypto::decrypt(ciphertext, key, nonce);

    assert(decrypted == plaintext);
    assert(ciphertext != plaintext);
    assert(nonce.size() == crypto_secretbox_NONCEBYTES);

    std::vector<std::uint8_t> secondNonce;
    const std::vector<std::uint8_t> secondCiphertext =
        crypto::encrypt(plaintext, key, secondNonce);
    assert(secondNonce != nonce);
    assert(secondCiphertext != ciphertext);

    const std::vector<std::uint8_t> emptyPlaintext;
    std::vector<std::uint8_t> emptyNonce;
    const std::vector<std::uint8_t> emptyCiphertext =
        crypto::encrypt(emptyPlaintext, key, emptyNonce);
    assert(
        crypto::decrypt(emptyCiphertext, key, emptyNonce)
        == emptyPlaintext);

    const std::vector<std::uint8_t> wrongKey =
        crypto::deriveKey("wrong-password", salt);
    bool rejected = false;
    try
    {
        static_cast<void>(crypto::decrypt(ciphertext, wrongKey, nonce));
    }
    catch (const std::runtime_error &)
    {
        rejected = true;
    }
    assert(rejected);

    bool invalidKeyRejected = false;
    try
    {
        static_cast<void>(crypto::encrypt(plaintext, {}, nonce));
    }
    catch (const std::runtime_error &)
    {
        invalidKeyRejected = true;
    }
    assert(invalidKeyRejected);

    std::vector<std::uint8_t> tamperedCiphertext = ciphertext;
    tamperedCiphertext.back() ^= 0x01U;
    rejected = false;
    try
    {
        static_cast<void>(crypto::decrypt(tamperedCiphertext, key, nonce));
    }
    catch (const std::runtime_error &)
    {
        rejected = true;
    }
    assert(rejected);

    std::cout << "Crypto tests passed.\n";
    return 0;
}
