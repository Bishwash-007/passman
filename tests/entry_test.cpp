#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "entry.hpp"

int main()
{
    const std::vector<entry::Entry> entries{
        {"example.com", "alice", "p@ssword", "https://example.com/work"},
        {"mail.example.com", "alice@mail.example.com", "second-secret",
         "personal account"},
        {"üñîçødé.example", "", "", "notes with symbols !@#$%^&*()"},
        {"long.example", std::string(1024, 'u'), std::string(2048, 'p'),
         std::string(4096, 'n')},
    };

    const std::vector<std::uint8_t> serialized = entry::serialize(entries);
    const std::vector<entry::Entry> restored =
        entry::deserialize(serialized);

    assert(restored == entries);

    bool truncatedRejected = false;
    try
    {
        std::vector<std::uint8_t> truncated = serialized;
        truncated.pop_back();
        static_cast<void>(entry::deserialize(truncated));
    }
    catch (const std::runtime_error &)
    {
        truncatedRejected = true;
    }
    assert(truncatedRejected);

    bool trailingDataRejected = false;
    try
    {
        std::vector<std::uint8_t> trailing = serialized;
        trailing.push_back(0xFFU);
        static_cast<void>(entry::deserialize(trailing));
    }
    catch (const std::runtime_error &)
    {
        trailingDataRejected = true;
    }
    assert(trailingDataRejected);

    bool invalidCountRejected = false;
    try
    {
        const std::vector<std::uint8_t> invalidCount{1U, 0U, 0U, 0U};
        static_cast<void>(entry::deserialize(invalidCount));
    }
    catch (const std::exception &)
    {
        invalidCountRejected = true;
    }
    assert(invalidCountRejected);

    std::cout << "Entry serialization tests passed.\n";
    return 0;
}
