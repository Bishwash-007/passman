#include "cli.hpp"

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <limits>

#include "password.hpp"

namespace cli
{
    namespace
    {
        std::size_t parseLength(const std::string &value)
        {
            std::size_t position = 0;
            unsigned long long parsed = 0;
            try
            {
                parsed = std::stoull(value, &position);
            }
            catch (const std::exception &)
            {
                throw std::runtime_error("Invalid password length: " + value);
            }
            if (position != value.size()
                || parsed > std::numeric_limits<std::size_t>::max())
            {
                throw std::runtime_error("Invalid password length: " + value);
            }
            return static_cast<std::size_t>(parsed);
        }
    }

    Arguments parse(
        int argc,
        char *argv[])
    {
        Arguments simpleArguments;

        if (argc < 2)
        {
            simpleArguments.command = Command::Interactive;
            return simpleArguments;
        }

        const std::string command = argv[1];

        if (command == "help" || command == "--help" || command == "-h")
        {
            simpleArguments.command = Command::Help;
            return simpleArguments;
        }

        if (command == "list")
        {
            simpleArguments.command = Command::List;
            return simpleArguments;
        }

        if (command == "lock")
        {
            simpleArguments.command = Command::Lock;
            return simpleArguments;
        }

        if (command == "generate")
        {
            if (argc > 3)
            {
                throw std::runtime_error("Usage: passman generate [length]");
            }
            Arguments arguments;
            arguments.command = Command::Generate;
            arguments.length = argc == 3
                ? parseLength(argv[2])
                : password::DEFAULT_GENERATED_LENGTH;
            return arguments;
        }

        if (command == "export" || command == "import")
        {
            if (argc != 3)
            {
                throw std::runtime_error(
                    "Usage: passman " + command + " <path>");
            }
            Arguments arguments;
            arguments.command = command == "export"
                ? Command::Export : Command::Import;
            arguments.path = argv[2];
            return arguments;
        }

        if (command == "add")
        {
            if (argc < 3)
            {
                throw std::runtime_error(
                    "Usage: passman add <site> [--generate]");
            }

            Arguments arguments;
            arguments.command = Command::Add;
            arguments.site = argv[2];

            if (argc >= 4)
            {
                if (std::string(argv[3]) == "--generate")
                {
                    arguments.generatePassword = true;
                }
                else
                {
                    throw std::runtime_error(
                        "Unknown option: " + std::string(argv[3]));
                }
            }

            return arguments;
        }

        if (command == "change-master")
        {
            Arguments arguments;
            arguments.command = Command::ChangeMaster;
            return arguments;
        }

        if (command == "get")
        {
            if (argc < 3)
            {
                throw std::runtime_error(
                    "Usage: passman get <site> [--copy]");
            }

            Arguments arguments;
            arguments.command = Command::Get;
            arguments.site = argv[2];

            if (argc >= 4)
            {
                if (std::string(argv[3]) == "--copy")
                {
                    arguments.copyPassword = true;
                }
                else
                {
                    throw std::runtime_error(
                        "Unknown option: " + std::string(argv[3]));
                }
            }

            return arguments;
        }

        if (command == "delete")
        {
            if (argc < 3)
            {
                throw std::runtime_error(
                    "Usage: passman delete <site>");
            }

            Arguments arguments;
            arguments.command = Command::Delete;
            arguments.site = argv[2];
            return arguments;
        }

        if (command == "search")
        {
            if (argc < 3)
            {
                throw std::runtime_error("Usage: passman search <query>");
            }

            Arguments searchArguments;
            searchArguments.command = Command::Search;
            searchArguments.query = argv[2];
            return searchArguments;
        }

        throw std::runtime_error("Unknown command: " + command);
    }

    Arguments parseLine(
        const std::string &line)
    {
        std::istringstream stream(line);

        std::vector<std::string> tokens;

        std::string token;

        while (stream >> token)
        {
            tokens.push_back(token);
        }

        if (tokens.empty())
        {
            return {};
        }

        const std::string &command =
            tokens[0];

        if (
            command == "help" ||
            command == "?")
        {
            Arguments arguments;
            arguments.command = Command::Help;
            return arguments;
        }

        if (command == "lock")
        {
            Arguments arguments;
            arguments.command = Command::Lock;
            return arguments;
        }

        if (command == "generate")
        {
            if (tokens.size() > 2)
            {
                throw std::runtime_error("Usage: generate [length]");
            }
            Arguments arguments;
            arguments.command = Command::Generate;
            arguments.length = tokens.size() == 2
                ? parseLength(tokens[1])
                : password::DEFAULT_GENERATED_LENGTH;
            return arguments;
        }

        if (command == "export" || command == "import")
        {
            if (tokens.size() != 2)
            {
                throw std::runtime_error(
                    "Usage: " + command + " <path>");
            }
            Arguments arguments;
            arguments.command = command == "export"
                ? Command::Export : Command::Import;
            arguments.path = tokens[1];
            return arguments;
        }

        if (command == "change-master")
        {
            Arguments arguments;
            arguments.command = Command::ChangeMaster;
            return arguments;
        }

        if (command == "list")
        {
            Arguments arguments;
            arguments.command = Command::List;
            return arguments;
        }

        if (command == "add")
        {

            if (tokens.size() < 2)
            {
                throw std::runtime_error(
                    "Usage: add <site> [--generate]");
            }

            Arguments arguments;

            arguments.command =
                Command::Add;

            arguments.site =
                tokens[1];

            for (
                std::size_t i = 2;
                i < tokens.size();
                ++i)
            {
                if (tokens[i] == "--generate")
                {
                    arguments.generatePassword = true;
                }
                else
                {
                    throw std::runtime_error(
                        "Unknown option: " +
                        tokens[i]);
                }
            }

            return arguments;
        }

        if (command == "get")
        {

            if (tokens.size() < 2)
            {
                throw std::runtime_error(
                    "Usage: get <site> [--copy]");
            }

            Arguments arguments;

            arguments.command =
                Command::Get;

            arguments.site =
                tokens[1];

            for (
                std::size_t i = 2;
                i < tokens.size();
                ++i)
            {
                if (tokens[i] == "--copy")
                {
                    arguments.copyPassword = true;
                }
                else
                {
                    throw std::runtime_error(
                        "Unknown option: " +
                        tokens[i]);
                }
            }

            return arguments;
        }

        if (command == "delete")
        {

            if (tokens.size() < 2)
            {
                throw std::runtime_error(
                    "Usage: delete <site>");
            }

            Arguments arguments;
            arguments.command = Command::Delete;
            arguments.site = tokens[1];
            return arguments;
        }

        if (command == "search")
        {
            if (tokens.size() < 2)
            {
                throw std::runtime_error("Usage: search <query>");
            }

            Arguments arguments;
            arguments.command = Command::Search;
            arguments.query = tokens[1];
            return arguments;
        }

        throw std::runtime_error(
            "Unknown command: " +
            command);
    }

    void printInteractiveUsage()
    {
        std::cout
            << "\nCommands:\n"
            << "  add <site>              Add a password entry\n"
            << "  add <site> --generate   Generate a password\n"
            << "  get <site>              Show entry\n"
            << "  get <site> --copy       Copy password\n"
            << "  list                    List sites\n"
            << "  delete <site>           Delete entry\n"
            << "  search <query>          Search entries\n"
            << "  change-master          Change the master password\n"
            << "  generate [length]      Generate a secure password\n"
            << "  export <path>          Export unencrypted entry data\n"
            << "  import <path>          Import entry data\n"
            << "  lock                    Lock vault\n"
            << "  help                    Show this help\n"
            << '\n';
    }

    void printUsage()
    {
        std::cout
            << "Passman - local password manager\n\n"
            << "Usage:\n"
            << "  passman\n"
            << "  passman add <site>\n"
            << "  passman add <site> --generate\n"
            << "  passman get <site>\n"
            << "  passman get <site> --copy\n"
            << "  passman list\n"
            << "  passman delete <site>\n"
            << "  passman search <query>\n"
            << "  passman generate [length]\n"
            << "  passman export <path>\n"
            << "  passman import <path>\n"
            << "  passman change-master\n"
            << "  passman lock\n"
            << "  passman --help\n"
            << '\n';
    }

}