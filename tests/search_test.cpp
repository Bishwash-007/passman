#include <cassert>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>

#include "commands.hpp"
#include "crypto.hpp"

int main()
{
    crypto::initialize();
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "passman-search-test.vault";
    std::error_code error;
    std::filesystem::remove(path, error);

    vault::Vault storedVault = vault::Vault::create(path.string(), "password");
    storedVault.add({"GitHub", "alice", "top-secret", "work repository"});
    storedVault.add({"github-actions", "ci", "another-secret",
                     "automation notes"});
    storedVault.add({"Mail", "alice@example.com", "mail-secret",
                     "https://mail.example.com"});
    storedVault.save();

    std::ostringstream output;
    std::streambuf *oldBuffer = std::cout.rdbuf(output.rdbuf());
    handleSearch(storedVault, "GITHUB");
    std::cout.rdbuf(oldBuffer);

    const std::string result = output.str();
    assert(result.find("GitHub") != std::string::npos);
    assert(result.find("github-actions") != std::string::npos);
    assert(result.find("top-secret") == std::string::npos);
    assert(result.find("another-secret") == std::string::npos);

    output.str("");
    output.clear();
    oldBuffer = std::cout.rdbuf(output.rdbuf());
    handleSearch(storedVault, "mail.example");
    std::cout.rdbuf(oldBuffer);
    assert(output.str().find("Mail") != std::string::npos);

    bool emptyRejected = false;
    try
    {
        handleSearch(storedVault, "");
    }
    catch (const std::runtime_error &)
    {
        emptyRejected = true;
    }
    assert(emptyRejected);

    std::filesystem::remove(path, error);
    std::cout << "Search tests passed.\n";
    return 0;
}
