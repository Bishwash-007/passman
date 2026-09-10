#include <cassert>
#include <iostream>
#include <stdexcept>

#include "cli.hpp"
#include "password.hpp"

int main()
{
    const cli::Arguments generated =
        cli::parseLine("generate 32");
    assert(generated.command == cli::Command::Generate);
    assert(generated.length == 32);

    const cli::Arguments defaultGenerated =
        cli::parseLine("generate");
    assert(defaultGenerated.length == password::DEFAULT_GENERATED_LENGTH);

    const cli::Arguments exportArguments =
        cli::parseLine("export export.pme");
    assert(exportArguments.command == cli::Command::Export);
    assert(exportArguments.path == "export.pme");

    const cli::Arguments importArguments =
        cli::parseLine("import export.pme");
    assert(importArguments.command == cli::Command::Import);

    bool invalidLengthRejected = false;
    try
    {
        static_cast<void>(cli::parseLine("generate invalid"));
    }
    catch (const std::runtime_error &)
    {
        invalidLengthRejected = true;
    }
    assert(invalidLengthRejected);

    bool missingPathRejected = false;
    try
    {
        static_cast<void>(cli::parseLine("export"));
    }
    catch (const std::runtime_error &)
    {
        missingPathRejected = true;
    }
    assert(missingPathRejected);

    std::cout << "CLI tests passed.\n";
    return 0;
}
