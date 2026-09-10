#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace entry
{

    struct Entry
    {
        std::string site;
        std::string username;
        std::string password;
        std::string notes;

        bool operator==(const Entry &) const = default;
    };

    std::vector<std::uint8_t> serialize(const std::vector<Entry> &entries);
    std::vector<Entry> deserialize(const std::vector<std::uint8_t> &data);

}