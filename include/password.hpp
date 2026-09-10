#pragma once

#include <cstddef>
#include <string>

namespace password
{
    constexpr std::size_t DEFAULT_GENERATED_LENGTH = 24;
    constexpr std::size_t MIN_GENERATED_LENGTH = 8;
    constexpr std::size_t MAX_GENERATED_LENGTH = 256;

    std::string readHidden(const std::string &prompt);
    std::string generate(std::size_t length);
}