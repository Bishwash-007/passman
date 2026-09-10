#pragma once

#include <string>
#include <vector>

namespace cli {

enum class Command
{
    Add,
    Get,
    List,
    Delete,
    Search,
    ChangeMaster,
    Generate,
    Export,
    Import,
    Lock,
    Help,
    Invalid
};

struct Arguments
{
    Command command = Command::Invalid;
    std::string site;
    std::string query;
    std::string path;
    std::size_t length = 0;
    bool generatePassword = false;
    bool copyPassword = false;
};

Arguments parse(
    int argc,
    char* argv[]
);

Arguments parseLine(
    const std::string& line
);

void printUsage();

void printInteractiveUsage();

}