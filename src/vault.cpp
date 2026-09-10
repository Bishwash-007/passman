#include "vault.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <utility>
#include <sys/stat.h>

#include <sodium.h>

#include "crypto.hpp"

namespace vault
{
    namespace
    {
        constexpr std::array<std::uint8_t, 4> MAGIC = {'P', 'M', 'V', '1'};

        void clearEntries(std::vector<entry::Entry> &entries)
        {
            for (entry::Entry &storedEntry : entries)
            {
                sodium_memzero(storedEntry.password.data(),
                               storedEntry.password.size());
                sodium_memzero(storedEntry.username.data(),
                               storedEntry.username.size());
                sodium_memzero(storedEntry.notes.data(),
                               storedEntry.notes.size());
            }
        }
    }

    bool vaultExists(const std::string &path)
    {
        std::ifstream file(path);

        return file.good();
    }

    RawVault loadRaw(const std::string &path)
    {
        std::ifstream file(path, std::ios::binary);

        if (!file)
        {
            throw std::runtime_error("Could not open vault file: " + path);
        }

        file.seekg(0, std::ios::end);

        const std::streamsize size = file.tellg();

        if (size < 0)
        {
            throw std::runtime_error("Could not determine vault file size");
        }

        file.seekg(0, std::ios::beg);

        std::vector<std::uint8_t> all(static_cast<std::size_t>(size));

        if (!all.empty())
        {
            file.read(reinterpret_cast<char *>(all.data()), size);
        }

        if (!file)
        {
            throw std::runtime_error("Failed to read vault file: " + path);
        }

        const std::size_t legacyHeaderSize = SALT_BYTES + NONCE_BYTES;
        const std::size_t currentHeaderSize = MAGIC.size() + 1 + legacyHeaderSize;
        if (all.size() < legacyHeaderSize)
        {
            throw std::runtime_error("Vault file too small / corrupted");
        }

        RawVault vault;
        std::size_t offset = 0;
        if (all.size() >= currentHeaderSize &&
            std::equal(MAGIC.begin(), MAGIC.end(), all.begin()))
        {
            vault.formatVersion = all[MAGIC.size()];
            if (vault.formatVersion != CURRENT_FORMAT_VERSION)
            {
                throw std::runtime_error("Unsupported vault format version");
            }
            offset = MAGIC.size() + 1;
        }

        const auto saltBegin = all.begin()
            + static_cast<std::vector<std::uint8_t>::difference_type>(offset);
        const auto saltEnd = saltBegin
            + static_cast<std::vector<std::uint8_t>::difference_type>(SALT_BYTES);
        vault.salt.assign(saltBegin, saltEnd);
        offset += SALT_BYTES;
        const auto nonceBegin = all.begin()
            + static_cast<std::vector<std::uint8_t>::difference_type>(offset);
        const auto nonceEnd = nonceBegin
            + static_cast<std::vector<std::uint8_t>::difference_type>(NONCE_BYTES);
        vault.nonce.assign(nonceBegin, nonceEnd);
        offset += NONCE_BYTES;
        vault.cipherText.assign(
            all.begin()
                + static_cast<std::vector<std::uint8_t>::difference_type>(offset),
            all.end());

        return vault;
    }

    void saveRaw(const std::string &path, const RawVault &vault)
    {
        if (vault.salt.size() != SALT_BYTES || vault.nonce.size() != NONCE_BYTES)
        {
            throw std::runtime_error("Invalid salt/nonce size");
        }

        const std::string temporaryPath = path + ".tmp";
        std::ofstream file(temporaryPath, std::ios::binary | std::ios::trunc);

        if (!file)
        {
            throw std::runtime_error("Could not write vault file: " + temporaryPath);
        }
        if (chmod(temporaryPath.c_str(), S_IRUSR | S_IWUSR) != 0)
        {
            file.close();
            std::filesystem::remove(temporaryPath);
            throw std::runtime_error("Could not secure temporary vault file");
        }

        file.write(reinterpret_cast<const char *>(MAGIC.data()),
                   static_cast<std::streamsize>(MAGIC.size()));
        file.put(static_cast<char>(CURRENT_FORMAT_VERSION));
        file.write(
            reinterpret_cast<const char *>(vault.salt.data()),
            static_cast<std::streamsize>(vault.salt.size()));

        file.write(
            reinterpret_cast<const char *>(vault.nonce.data()),
            static_cast<std::streamsize>(vault.nonce.size()));

        file.write(
            reinterpret_cast<const char *>(vault.cipherText.data()),
            static_cast<std::streamsize>(vault.cipherText.size()));

        if (!file)
        {
            file.close();
            std::filesystem::remove(temporaryPath);
            throw std::runtime_error("Failed while writing vault file: " + path);
        }

        file.close();
        std::error_code error;
        std::filesystem::rename(temporaryPath, path, error);
        if (error)
        {
            std::filesystem::remove(temporaryPath);
            throw std::runtime_error("Could not atomically replace vault file: "
                                     + error.message());
        }
        if (chmod(path.c_str(), S_IRUSR | S_IWUSR) != 0)
        {
            throw std::runtime_error("Could not secure vault file");
        }
    }

    // Vault

    Vault::Vault(
        std::string path,
        std::vector<std::uint8_t> salt,
        std::vector<std::uint8_t> key,
        std::vector<entry::Entry> entries)
        : path_(std::move(path)),
          salt_(std::move(salt)),
          key_(std::move(key)),
          entries_(std::move(entries))
    {
    }

    Vault::Vault(Vault &&other) noexcept
        : path_(std::move(other.path_)),
          salt_(std::move(other.salt_)),
          key_(std::move(other.key_)),
          entries_(std::move(other.entries_))
    {
    }

    Vault &Vault::operator=(Vault &&other) noexcept
    {
        if (this != &other)
        {
            if (!key_.empty())
            {
                sodium_memzero(key_.data(), key_.size());
            }

            path_ = std::move(other.path_);
            salt_ = std::move(other.salt_);
            key_ = std::move(other.key_);
            entries_ = std::move(other.entries_);
        }

        return *this;
    }

    Vault Vault::create(const std::string &path, const std::string &masterPassword)
    {
        if (masterPassword.empty())
        {
            throw std::runtime_error("Master password cannot be empty");
        }

        // Generate random salt
        std::vector<std::uint8_t> salt(SALT_BYTES);

        randombytes_buf(salt.data(), salt.size());

        // Derive encryption key
        std::vector<std::uint8_t> key = crypto::deriveKey(masterPassword, salt);

        // Start with an empty vault
        std::vector<entry::Entry> entries;

        Vault vault(path, std::move(salt), std::move(key), std::move(entries));

        // Immediately create encrypted vault.dat
        vault.save();

        return vault;
    }

    Vault Vault::open(const std::string &path, const std::string &masterPassword)
    {
        if (masterPassword.empty())
        {
            throw std::runtime_error("Master password cannot be empty");
        }

        RawVault rawVault = loadRaw(path);
        std::vector<std::uint8_t> key;
        std::vector<std::uint8_t> plaintext;
        try
        {
            key = crypto::deriveKey(masterPassword, rawVault.salt);
            plaintext = crypto::decrypt(
                rawVault.cipherText,
                key,
                rawVault.nonce);

            std::vector<entry::Entry> entries =
                entry::deserialize(plaintext);
            sodium_memzero(plaintext.data(), plaintext.size());
            return Vault(
                path,
                std::move(rawVault.salt),
                std::move(key),
                std::move(entries));
        }
        catch (...)
        {
            sodium_memzero(plaintext.data(), plaintext.size());
            sodium_memzero(key.data(), key.size());
            throw;
        }
    }

    void Vault::add(const entry::Entry &newEntry)
    {
        if (newEntry.site.empty())
        {
            throw std::runtime_error("Site cannot be empty");
        }

        // V1: one entry per site
        for (const entry::Entry &existing : entries_)
        {
            if (existing.site == newEntry.site)
            {
                throw std::runtime_error("An entry for this site already exists");
            }
        }

        entries_.push_back(newEntry);
    }

    const entry::Entry &Vault::get(const std::string &site) const
    {
        for (const entry::Entry &entry : entries_)
        {
            if (entry.site == site)
            {
                return entry;
            }
        }

        throw std::runtime_error("No entry found for site: " + site);
    }

    const std::vector<entry::Entry> &Vault::list() const
    {
        return entries_;
    }

    void Vault::remove(const std::string &site)
    {
        for (auto iterator = entries_.begin(); iterator != entries_.end(); ++iterator)
        {
            if (iterator->site == site)
            {
                entries_.erase(iterator);
                return;
            }
        }

        throw std::runtime_error("No entry found for site: " + site);
    }

    void Vault::save()
    {
        std::vector<std::uint8_t> plaintext = entry::serialize(entries_);
        try
        {
            std::vector<std::uint8_t> nonce;
            std::vector<std::uint8_t> ciphertext =
                crypto::encrypt(plaintext, key_, nonce);
            RawVault rawVault;
            rawVault.salt = salt_;
            rawVault.nonce = std::move(nonce);
            rawVault.cipherText = std::move(ciphertext);
            saveRaw(path_, rawVault);
        }
        catch (...)
        {
            sodium_memzero(plaintext.data(), plaintext.size());
            throw;
        }
        sodium_memzero(plaintext.data(), plaintext.size());
    }

    void Vault::importEntries(const std::vector<entry::Entry> &newEntries)
    {
        std::vector<entry::Entry> combined = entries_;
        for (const entry::Entry &newEntry : newEntries)
        {
            if (newEntry.site.empty())
            {
                throw std::runtime_error("Imported site cannot be empty");
            }
            for (const entry::Entry &existing : combined)
            {
                if (existing.site == newEntry.site)
                {
                    throw std::runtime_error(
                        "An entry for this site already exists");
                }
            }
            combined.push_back(newEntry);
        }

        std::vector<entry::Entry> original = entries_;
        entries_ = std::move(combined);
        try
        {
            save();
        }
        catch (...)
        {
            clearEntries(entries_);
            entries_ = original;
            throw;
        }
        clearEntries(original);
    }

    void Vault::changeMasterPassword(const std::string &newPassword)
    {
        if (newPassword.empty())
        {
            throw std::runtime_error("Master password cannot be empty");
        }

        std::vector<std::uint8_t> newSalt(SALT_BYTES);
        std::vector<std::uint8_t> newKey;
        std::vector<std::uint8_t> plaintext;

        try
        {
            randombytes_buf(newSalt.data(), newSalt.size());
            newKey = crypto::deriveKey(newPassword, newSalt);
            plaintext = entry::serialize(entries_);
            std::vector<std::uint8_t> newNonce;
            std::vector<std::uint8_t> newCiphertext =
                crypto::encrypt(plaintext, newKey, newNonce);

            RawVault rawVault;
            rawVault.salt = newSalt;
            rawVault.nonce = std::move(newNonce);
            rawVault.cipherText = std::move(newCiphertext);
            saveRaw(path_, rawVault);
        }
        catch (...)
        {
            sodium_memzero(plaintext.data(), plaintext.size());
            sodium_memzero(newKey.data(), newKey.size());
            throw;
        }

        sodium_memzero(plaintext.data(), plaintext.size());
        sodium_memzero(key_.data(), key_.size());
        sodium_memzero(salt_.data(), salt_.size());
        key_ = std::move(newKey);
        salt_ = std::move(newSalt);
    }

    void Vault::changeMasterPassword(
        const std::string &currentPassword,
        const std::string &newPassword)
    {
        if (currentPassword.empty())
        {
            throw std::runtime_error("Current master password cannot be empty");
        }

        std::vector<std::uint8_t> currentKey =
            crypto::deriveKey(currentPassword, salt_);
        const bool matches =
            currentKey.size() == key_.size() &&
            sodium_memcmp(currentKey.data(), key_.data(), key_.size()) == 0;
        sodium_memzero(currentKey.data(), currentKey.size());

        if (!matches)
        {
            throw std::runtime_error("Current master password is incorrect");
        }

        changeMasterPassword(newPassword);
    }

    Vault::~Vault()
    {
        if (!key_.empty())
        {
            sodium_memzero(key_.data(), key_.size());
        }

        clearEntries(entries_);
    }

}