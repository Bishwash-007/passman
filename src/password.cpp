#include "password.hpp"

#include <cstdint>
#include <array>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include <sodium.h>

#include <termios.h>
#include <unistd.h>

namespace password
{

    std::string readHidden(const std::string &prompt)
    {
        std::cout << prompt;
        std::cout.flush();

        termios oldSettings{};

        if (tcgetattr(STDIN_FILENO, &oldSettings) != 0)
        {
            throw std::runtime_error("Could not read terminal settings");
        }

        termios newSettings = oldSettings;
        newSettings.c_lflag &= static_cast<tcflag_t>(~ECHO);

        if (tcsetattr(STDIN_FILENO, TCSANOW, &newSettings) != 0)
        {
            throw std::runtime_error(
                "Could not disable terminal echo");
        }

        std::string value;
        std::getline(std::cin, value);

        // Restore terminal echo.
        if (tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings) != 0)
        {
            throw std::runtime_error("Could not restore terminal settings");
        }

        std::cout << '\n';

        return value;
    }

    std::string generate(std::size_t length)
    {
        if (length < MIN_GENERATED_LENGTH)
        {
            throw std::runtime_error("Generated password length must be at least "
                                     + std::to_string(MIN_GENERATED_LENGTH));
        }
        if (length > MAX_GENERATED_LENGTH)
        {
            throw std::runtime_error("Generated password length cannot exceed "
                                     + std::to_string(MAX_GENERATED_LENGTH));
        }

        constexpr std::string_view uppercase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        constexpr std::string_view lowercase = "abcdefghijklmnopqrstuvwxyz";
        constexpr std::string_view digits = "0123456789";
        constexpr std::string_view symbols = "!@#$%^&*()-_=+[]{}:,.?";
        const std::array<std::string_view, 4> classes{
            uppercase, lowercase, digits, symbols};
        std::string result;
        result.reserve(length);

        for (const std::string_view characterClass : classes)
        {
            result.push_back(characterClass[
                randombytes_uniform(static_cast<std::uint32_t>(
                    characterClass.size()))]);
        }
        constexpr std::string_view alphabet =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789"
            "!@#$%^&*()-_=+[]{}:,.?";
        while (result.size() < length)
        {
            result.push_back(alphabet[randombytes_uniform(
                static_cast<std::uint32_t>(alphabet.size()))]);
        }

        for (std::size_t i = result.size(); i > 1; --i)
        {
            const std::size_t index = randombytes_uniform(
                static_cast<std::uint32_t>(i));
            std::swap(result[i - 1], result[index]);
        }
        return result;
    }

}