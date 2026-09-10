#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>

#include "crypto.hpp"
#include "export.hpp"
#include "vault.hpp"

namespace
{
    bool rawVaultsEqual(
        const vault::RawVault &left,
        const vault::RawVault &right)
    {
        return left.salt == right.salt
            && left.nonce == right.nonce
            && left.cipherText == right.cipherText
            && left.formatVersion == right.formatVersion;
    }

    class TemporaryVaultPath
    {
    public:
        TemporaryVaultPath()
            : path_(
                  std::filesystem::temp_directory_path()
                  / ("passman-test-" + std::to_string(
                         static_cast<unsigned long long>(
                             std::filesystem::file_time_type::clock::now()
                                 .time_since_epoch()
                                 .count()))))
        {
        }

        ~TemporaryVaultPath()
        {
            std::error_code error;
            std::filesystem::remove(path_, error);
            std::filesystem::remove(path_.string() + ".tmp", error);
            std::filesystem::remove(path_.string() + ".export", error);
            std::filesystem::remove(path_.string() + ".imported", error);
            std::filesystem::remove(path_.string() + ".imported.tmp", error);
            std::filesystem::remove(path_.string() + ".malformed", error);
        }

        const std::filesystem::path &path() const
        {
            return path_;
        }

    private:
        std::filesystem::path path_;
    };

    void assertEntries(const vault::Vault &storedVault)
    {
        const auto &entries = storedVault.list();
        assert(entries.size() == 2);
        assert(entries[0].site == "example.com");
        assert(entries[0].username == "alice");
        assert(entries[0].password == "first-secret");
        assert(entries[1].site == "mail.example.com");
        assert(entries[1].notes == "personal account");
    }
}

int main()
{
    crypto::initialize();
    TemporaryVaultPath temporaryVault;
    const std::string path = temporaryVault.path().string();
    const std::string oldPassword = "old-password";
    const std::string newPassword = "new-password";
    const std::string thirdPassword = "third-password";

    vault::Vault storedVault = vault::Vault::create(path, oldPassword);
    assert(vault::vaultExists(path));
    struct stat vaultStatus{};
    assert(stat(path.c_str(), &vaultStatus) == 0);
    assert((vaultStatus.st_mode & (S_IRWXG | S_IRWXO)) == 0);
    assert(vault::loadRaw(path).salt.size() == vault::SALT_BYTES);
    assert(vault::loadRaw(path).nonce.size() == vault::NONCE_BYTES);
    storedVault.add({"example.com", "alice", "first-secret", "work account"});
    storedVault.add(
        {"mail.example.com", "alice-mail", "second-secret", "personal account"});
    storedVault.save();
    assertEntries(storedVault);
    assert(!std::filesystem::exists(path + ".tmp"));
    const vault::RawVault beforeRotation = vault::loadRaw(path);

    bool duplicateRejected = false;
    try
    {
        storedVault.add({"example.com", "other", "other-secret", ""});
    }
    catch (const std::runtime_error &)
    {
        duplicateRejected = true;
    }
    assert(duplicateRejected);
    assert(storedVault.get("example.com").password == "first-secret");

    storedVault.remove("mail.example.com");
    assert(storedVault.list().size() == 1);
    storedVault.add(
        {"mail.example.com", "alice-mail", "second-secret", "personal account"});
    assertEntries(storedVault);
    storedVault.save();
    const vault::RawVault beforePasswordRotation = vault::loadRaw(path);

    bool incorrectCurrentRejected = false;
    try
    {
        storedVault.changeMasterPassword("incorrect-password", newPassword);
    }
    catch (const std::runtime_error &)
    {
        incorrectCurrentRejected = true;
    }
    assert(incorrectCurrentRejected);
    assert(rawVaultsEqual(
        beforePasswordRotation,
        vault::loadRaw(path)));

    bool emptyNewPasswordRejected = false;
    try
    {
        storedVault.changeMasterPassword(oldPassword, "");
    }
    catch (const std::runtime_error &)
    {
        emptyNewPasswordRejected = true;
    }
    assert(emptyNewPasswordRejected);
    assert(rawVaultsEqual(
        beforePasswordRotation,
        vault::loadRaw(path)));

    storedVault.changeMasterPassword(oldPassword, newPassword);
    assertEntries(storedVault);
    const vault::RawVault afterRotation = vault::loadRaw(path);
    assert(afterRotation.salt != beforePasswordRotation.salt);
    assert(afterRotation.nonce != beforePasswordRotation.nonce);
    assert(afterRotation.cipherText != beforePasswordRotation.cipherText);

    bool oldPasswordRejected = false;
    try
    {
        static_cast<void>(vault::Vault::open(path, oldPassword));
    }
    catch (const std::runtime_error &)
    {
        oldPasswordRejected = true;
    }
    assert(oldPasswordRejected);

    vault::Vault reopenedVault = vault::Vault::open(path, newPassword);
    assertEntries(reopenedVault);

    bool oldPasswordRejectedAgain = false;
    try
    {
        static_cast<void>(vault::Vault::open(path, oldPassword));
    }
    catch (const std::runtime_error &)
    {
        oldPasswordRejectedAgain = true;
    }
    assert(oldPasswordRejectedAgain);

    reopenedVault.changeMasterPassword(newPassword, thirdPassword);
    const vault::RawVault afterSecondRotation = vault::loadRaw(path);
    assert(afterSecondRotation.salt != afterRotation.salt);
    assert(afterSecondRotation.nonce != afterRotation.nonce);
    assert(afterSecondRotation.cipherText != afterRotation.cipherText);
    vault::Vault thirdVault = vault::Vault::open(path, thirdPassword);
    assertEntries(thirdVault);

    const std::filesystem::path exportPath =
        temporaryVault.path().string() + ".export";
    export_format::write(exportPath.string(), thirdVault.list());
    assert(export_format::read(exportPath.string()) == thirdVault.list());

    bool existingExportRejected = false;
    try
    {
        export_format::write(exportPath.string(), thirdVault.list());
    }
    catch (const std::runtime_error &)
    {
        existingExportRejected = true;
    }
    assert(existingExportRejected);

    vault::Vault importedVault = vault::Vault::create(
        temporaryVault.path().string() + ".imported", "import-password");
    importedVault.importEntries(thirdVault.list());
    assertEntries(importedVault);

    bool duplicateImportRejected = false;
    try
    {
        importedVault.importEntries(thirdVault.list());
    }
    catch (const std::runtime_error &)
    {
        duplicateImportRejected = true;
    }
    assert(duplicateImportRejected);
    assertEntries(importedVault);

    const std::filesystem::path malformedExport =
        temporaryVault.path().string() + ".malformed";
    {
        std::ofstream file(malformedExport, std::ios::binary);
        file << "not-an-export";
    }
    bool malformedExportRejected = false;
    try
    {
        static_cast<void>(export_format::read(malformedExport.string()));
    }
    catch (const std::runtime_error &)
    {
        malformedExportRejected = true;
    }
    assert(malformedExportRejected);

    bool secondPasswordRejected = false;
    try
    {
        static_cast<void>(vault::Vault::open(path, newPassword));
    }
    catch (const std::runtime_error &)
    {
        secondPasswordRejected = true;
    }
    assert(secondPasswordRejected);

    const vault::RawVault raw = vault::loadRaw(path);
    {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        assert(file);
        file.put('P');
        file.put('M');
        file.put('V');
        file.put('1');
        file.put(static_cast<char>(vault::CURRENT_FORMAT_VERSION + 1));
        file.write(
            reinterpret_cast<const char *>(raw.salt.data()),
            static_cast<std::streamsize>(raw.salt.size()));
        file.write(
            reinterpret_cast<const char *>(raw.nonce.data()),
            static_cast<std::streamsize>(raw.nonce.size()));
        file.write(
            reinterpret_cast<const char *>(raw.cipherText.data()),
            static_cast<std::streamsize>(raw.cipherText.size()));
    }

    bool unsupportedVersionRejected = false;
    try
    {
        static_cast<void>(vault::Vault::open(path, newPassword));
    }
    catch (const std::runtime_error &error)
    {
        unsupportedVersionRejected =
            std::string(error.what()).find("Unsupported vault format version")
            != std::string::npos;
    }
    assert(unsupportedVersionRejected);

    {
        std::ofstream invalidHeader(path, std::ios::binary | std::ios::trunc);
        assert(invalidHeader);
        invalidHeader.put('X');
        invalidHeader.put('M');
        invalidHeader.put('V');
        invalidHeader.put('1');
        invalidHeader.put(static_cast<char>(vault::CURRENT_FORMAT_VERSION));
        invalidHeader.write(
            reinterpret_cast<const char *>(raw.salt.data()),
            static_cast<std::streamsize>(raw.salt.size()));
        invalidHeader.write(
            reinterpret_cast<const char *>(raw.nonce.data()),
            static_cast<std::streamsize>(raw.nonce.size()));
        invalidHeader.write(
            reinterpret_cast<const char *>(raw.cipherText.data()),
            static_cast<std::streamsize>(raw.cipherText.size()));
    }

    bool invalidHeaderRejected = false;
    try
    {
        static_cast<void>(vault::Vault::open(path, thirdPassword));
    }
    catch (const std::runtime_error &)
    {
        invalidHeaderRejected = true;
    }
    assert(invalidHeaderRejected);

    std::ofstream truncatedFile(path, std::ios::binary | std::ios::trunc);
    assert(truncatedFile);
    truncatedFile.put('P');
    truncatedFile.put('M');
    truncatedFile.put('V');
    truncatedFile.put('1');
    truncatedFile.put(static_cast<char>(vault::CURRENT_FORMAT_VERSION));
    truncatedFile.close();

    bool corruptedVaultRejected = false;
    try
    {
        static_cast<void>(vault::Vault::open(path, newPassword));
    }
    catch (const std::runtime_error &)
    {
        corruptedVaultRejected = true;
    }
    assert(corruptedVaultRejected);

    vault::saveRaw(path, afterSecondRotation);
    vault::RawVault validRaw = vault::loadRaw(path);
    validRaw.cipherText.back() ^= 0x01U;
    vault::saveRaw(path, validRaw);

    bool tamperedVaultRejected = false;
    try
    {
        static_cast<void>(vault::Vault::open(path, newPassword));
    }
    catch (const std::runtime_error &)
    {
        tamperedVaultRejected = true;
    }
    assert(tamperedVaultRejected);

    vault::saveRaw(path, afterSecondRotation);
    bool invalidRawRejected = false;
    try
    {
        vault::RawVault invalidRaw = afterSecondRotation;
        invalidRaw.salt.clear();
        vault::saveRaw(path, invalidRaw);
    }
    catch (const std::runtime_error &)
    {
        invalidRawRejected = true;
    }
    assert(invalidRawRejected);
    assert(rawVaultsEqual(afterSecondRotation, vault::loadRaw(path)));
    assert(!rawVaultsEqual(beforeRotation, afterRotation));

    std::cout << "Vault tests passed.\n";
    return 0;
}
