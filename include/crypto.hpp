#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace crypto
{

    constexpr std::size_t KEY_BYTES = 32;

    void initialize();

    std::vector<std::uint8_t> deriveKey(
        const std::string &password,
        const std::vector<std::uint8_t> &salt);

    std::vector<std::uint8_t> encrypt(
        const std::vector<std::uint8_t> &plaintext,
        const std::vector<std::uint8_t> &key,
        std::vector<std::uint8_t> &nonce);

    std::vector<std::uint8_t> decrypt(
        const std::vector<std::uint8_t> &ciphertext,
        const std::vector<std::uint8_t> &key,
        const std::vector<std::uint8_t> &nonce);

}