#include "session.hpp"

#include <iostream>
#include <poll.h>
#include <stdexcept>
#include <unistd.h>
#include <utility>

#include "commands.hpp"
#include "clipboard_timer.hpp"
#include "password.hpp"

namespace session
{

    Session::Session(
        vault::Vault vault)
        : vault_(std::move(vault)),
          lastActivity_(
              std::chrono::steady_clock::now()),
          timeout_(
              std::chrono::seconds(30))
    {
    }

    Session::~Session()
    {
        clipboard_timer::cancel();
    }

    void Session::updateActivity()
    {
        lastActivity_ =
            std::chrono::steady_clock::now();
    }

    bool Session::shouldAutoLock() const
    {
        const auto now =
            std::chrono::steady_clock::now();

        return (
            now - lastActivity_ >= timeout_);
    }

    void Session::lock()
    {
        clipboard_timer::cancel();
        std::cout
            << "\nVault locked.\n";
    }

    void Session::processCommand(
        const cli::Arguments &arguments)
    {
        switch (arguments.command)
        {

        case cli::Command::Add:
            handleAdd(
                vault_,
                arguments.site,
                arguments.generatePassword);
            break;

        case cli::Command::Get:
            handleGet(
                vault_,
                arguments.site,
                arguments.copyPassword);
            break;

        case cli::Command::List:
            handleList(vault_);
            break;

        case cli::Command::Delete:
            handleDelete(
                vault_,
                arguments.site);
            break;

        case cli::Command::Search:
            handleSearch(vault_, arguments.query);
            break;

        case cli::Command::ChangeMaster:
            handleChangeMaster(vault_);
            break;

        case cli::Command::Generate:
            handleGenerate(
                arguments.length == 0
                    ? password::DEFAULT_GENERATED_LENGTH
                    : arguments.length);
            break;

        case cli::Command::Export:
            handleExport(vault_, arguments.path);
            break;

        case cli::Command::Import:
            handleImport(vault_, arguments.path);
            break;

        case cli::Command::Help:
            cli::printInteractiveUsage();
            break;

        case cli::Command::Lock:
            lock();
            break;

        case cli::Command::Interactive:
            break;

        case cli::Command::Invalid:
            break;
        }
    }

    void Session::run()
    {
        std::cout
            << "\nVault unlocked.\n"
            << "Type 'help' for commands.\n\n";

        bool promptDisplayed = false;

        while (true)
        {

            if (shouldAutoLock())
            {

                std::cout
                    << "\nSession timed out.\n";

                lock();
                return;
            }

            if (!promptDisplayed)
            {
                std::cout << "passman> ";
                std::cout.flush();
                promptDisplayed = true;
            }

            pollfd input{};
            input.fd = STDIN_FILENO;
            input.events = POLLIN;
            const int result = poll(&input, 1, 1000);
            if (result < 0)
            {
                throw std::runtime_error("Failed while waiting for input");
            }
            if (result == 0)
            {
                continue;
            }
            if ((input.revents & (POLLIN | POLLHUP)) == 0)
            {
                continue;
            }

            std::string line;
            if (!std::getline(std::cin, line))
            {

                std::cout << '\n';
                lock();
                return;
            }

            if (line.empty())
            {
                promptDisplayed = false;
                continue;
            }

            try
            {

                const cli::Arguments arguments =
                    cli::parseLine(line);

                if (
                    arguments.command == cli::Command::Lock)
                {
                    lock();
                    return;
                }

                processCommand(arguments);

                updateActivity();
                promptDisplayed = false;
            }
            catch (const std::exception &e)
            {
                promptDisplayed = false;

                std::cerr
                    << "ERROR: "
                    << e.what()
                    << '\n';
            }

            std::cout << '\n';
        }
    }

}