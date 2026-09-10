#include <iostream>
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

        constexpr const char *vaultPath = "vault.dat";
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

        session::Session unlockedSession(std::move(vault));
        unlockedSession.run();
    }
    catch (const std::exception &error)
    {
        std::cerr << "ERROR: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
