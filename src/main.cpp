#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <utility>

#include <sodium.h>

#include "cli.hpp"
#include "crypto.hpp"
#include "password.hpp"
#include "session.hpp"
#include "vault.hpp"

int main(int argc, char *argv[])
{
    try
    {
        crypto::initialize();

        const cli::Arguments arguments = cli::parse(argc, argv);
        if (arguments.command == cli::Command::Help)
        {
            cli::printUsage();
            return 0;
        }
        if (arguments.command == cli::Command::Invalid)
        {
            return 1;
        }

        const char *homeDirectory = std::getenv("HOME");
        if (homeDirectory == nullptr || homeDirectory[0] == '\0')
        {
            throw std::runtime_error(
                "HOME environment variable is not set; cannot determine vault location");
        }

        const std::filesystem::path vaultDirectory =
            std::filesystem::path(homeDirectory) / ".local" / "share" / "passman";
        std::filesystem::create_directories(vaultDirectory);
        const std::filesystem::path vaultPath = vaultDirectory / "vault.dat";
        std::string masterPassword = password::readHidden("Master password: ");

        vault::Vault vault = [&]() {
            try
            {
                return vault::vaultExists(vaultPath)
                    ? vault::Vault::open(vaultPath, masterPassword)
                    : vault::Vault::create(vaultPath, masterPassword);
            }
            catch (...)
            {
                sodium_memzero(
                    masterPassword.data(),
                    masterPassword.size());
                throw;
            }
        }();

        sodium_memzero(masterPassword.data(), masterPassword.size());

        if (arguments.command == cli::Command::Interactive)
        {
            session::Session unlockedSession(std::move(vault));
            unlockedSession.run();
            return 0;
        }

        session::Session unlockedSession(std::move(vault));
        unlockedSession.processCommand(arguments);
    }
    catch (const std::exception &error)
    {
        std::cerr << "ERROR: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
