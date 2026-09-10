#include "commands.hpp"

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

#include <sodium.h>

#include "clipboard.hpp"
#include "clipboard_timer.hpp"
#include "export.hpp"
#include "password.hpp"

namespace
{
    std::string readLine(const std::string &prompt)
    {
        std::cout << prompt;
        std::string value;
        if (!std::getline(std::cin, value))
        {
            throw std::runtime_error("Input closed");
        }
        return value;
    }
}

void handleAdd(vault::Vault &vault, const std::string &site, bool generatePassword)
{
    std::cout << "Adding entry for: " << site << "\n\n";
    const std::string username = readLine("Username: ");
    std::string entryPassword = generatePassword
        ? password::generate(24)
        : password::readHidden("Password: ");

    if (generatePassword)
    {
        std::cout << "Generated password and stored it securely.\n";
    }

    const std::string notes = readLine("Notes: ");
    vault.add({site, username, entryPassword, notes});
    vault.save();
    sodium_memzero(entryPassword.data(), entryPassword.size());
    std::cout << "\nEntry added successfully.\n";
}

void handleGet(const vault::Vault &vault, const std::string &site, bool copyPassword)
{
    const auto &storedEntry = vault.get(site);
    std::cout << "\nSite:     " << storedEntry.site << '\n'
              << "Username: " << storedEntry.username << '\n';

    if (copyPassword)
    {
        constexpr unsigned int clipboardTimeout = 30;
        clipboard::copy(storedEntry.password);
        clipboard_timer::clearAfter(storedEntry.password, clipboardTimeout);
        std::cout << "Password copied to clipboard.\n"
                  << "Clipboard will be cleared in "
                  << clipboardTimeout << " seconds.\n";
    }
    else
    {
        std::cout << "Password: ********\n";
    }

    std::cout << "Notes:    " << storedEntry.notes << '\n';
}

void handleList(const vault::Vault &vault)
{
    const auto &entries = vault.list();
    if (entries.empty())
    {
        std::cout << "Vault is empty.\n";
        return;
    }

    std::cout << "Saved entries:\n\n";
    for (const auto &storedEntry : entries)
    {
        std::cout << "- " << storedEntry.site << '\n';
    }
}

void handleDelete(vault::Vault &vault, const std::string &site)
{
    vault.remove(site);
    vault.save();
    std::cout << "Entry deleted successfully.\n";
}

void handleSearch(const vault::Vault &vault, const std::string &query)
{
    std::string needle = query;
    std::transform(needle.begin(), needle.end(), needle.begin(),
                   [](unsigned char character) {
                       return static_cast<char>(std::tolower(character));
                   });

    if (needle.empty())
    {
        throw std::runtime_error("Search query cannot be empty");
    }

    struct Match
    {
        int rank;
        const entry::Entry *entry;
    };
    std::vector<Match> matches;
    for (const auto &storedEntry : vault.list())
    {
        std::string haystack = storedEntry.site + " "
            + storedEntry.username + " " + storedEntry.notes;
        std::transform(haystack.begin(), haystack.end(), haystack.begin(),
                       [](unsigned char character) {
                           return static_cast<char>(std::tolower(character));
                       });

        const std::size_t position = haystack.find(needle);
        if (position != std::string::npos)
        {
            std::string normalizedSite = storedEntry.site;
            std::transform(
                normalizedSite.begin(), normalizedSite.end(),
                normalizedSite.begin(), [](unsigned char character) {
                    return static_cast<char>(std::tolower(character));
                });
            const int rank = normalizedSite == needle ? 0
                : normalizedSite.rfind(needle, 0) == 0 ? 1 : 2;
            matches.push_back({rank, &storedEntry});
        }
    }

    std::stable_sort(matches.begin(), matches.end(),
                     [](const Match &left, const Match &right) {
                         return left.rank < right.rank;
                     });
    for (const Match &match : matches)
    {
        std::cout << "- " << match.entry->site << '\n';
    }
    if (matches.empty())
    {
        std::cout << "No matching entries.\n";
    }
}

void handleGenerate(std::size_t length)
{
    std::cout << password::generate(length) << '\n';
}

void handleExport(const vault::Vault &vault, const std::string &path)
{
    std::cout << "WARNING: export data is unencrypted.\n";
    export_format::write(path, vault.list());
    std::cout << "Exported " << vault.list().size()
              << " entries to " << path << ".\n";
}

void handleImport(vault::Vault &vault, const std::string &path)
{
    const std::vector<entry::Entry> imported = export_format::read(path);
    vault.importEntries(imported);
    std::cout << "Imported " << imported.size() << " entries.\n";
}

void handleChangeMaster(vault::Vault &vault)
{
    std::string newPassword = password::readHidden("New master password: ");
    std::string confirmation;

    try
    {
        confirmation = password::readHidden("Confirm new master password: ");
        if (newPassword != confirmation)
        {
            throw std::runtime_error("Master passwords do not match");
        }

        vault.changeMasterPassword(newPassword);
    }
    catch (...)
    {
        sodium_memzero(newPassword.data(), newPassword.size());
        sodium_memzero(confirmation.data(), confirmation.size());
        throw;
    }

    sodium_memzero(newPassword.data(), newPassword.size());
    sodium_memzero(confirmation.data(), confirmation.size());
    std::cout << "Master password changed.\n";
}
