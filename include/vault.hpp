#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <sodium.h>

#include "entry.hpp"

namespace vault
{
    constexpr std::size_t SALT_BYTES = crypto_pwhash_SALTBYTES;
    constexpr std::size_t NONCE_BYTES = crypto_secretbox_NONCEBYTES;
    constexpr std::uint8_t CURRENT_FORMAT_VERSION = 1;

    struct RawVault
    {
        std::vector<std::uint8_t> salt;
        std::vector<std::uint8_t> nonce;
        std::vector<std::uint8_t> cipherText;
        std::uint8_t formatVersion = CURRENT_FORMAT_VERSION;
    };

    bool vaultExists(const std::string &path);
    RawVault loadRaw(const std::string &path);
    void saveRaw(const std::string &path, const RawVault &vault);

    class Vault
    {
    public:
        static Vault create(const std::string &path, const std::string &masterPassword);
        static Vault open(const std::string &path, const std::string &masterPassword);

        ~Vault();

        // Vault contains sensitive key material.
        // Do not allow accidental copies.
        Vault(const Vault &) = delete;
        Vault &operator=(const Vault &) = delete;

        // Moving is allowed.
        Vault(Vault &&) noexcept;
        Vault &operator=(Vault &&) noexcept;

        void add(const entry::Entry &entry);

        const entry::Entry &get(const std::string &site) const;
        const std::vector<entry::Entry> &list() const;

        void remove(const std::string &site);
        void save();
        void importEntries(const std::vector<entry::Entry> &entries);
        void changeMasterPassword(const std::string &newPassword);
        void changeMasterPassword(const std::string &currentPassword, const std::string &newPassword);

    private:
        Vault(std::string path, std::vector<std::uint8_t> salt, std::vector<std::uint8_t> key, std::vector<entry::Entry> entries);

        std::string path_;

        std::vector<std::uint8_t> salt_;
        std::vector<std::uint8_t> key_;

        std::vector<entry::Entry> entries_;
        
    };

}