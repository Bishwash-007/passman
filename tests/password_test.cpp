#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

#include "password.hpp"

namespace
{
    bool hasCharacterFrom(const std::string &value, const std::string &characters)
    {
        for (const char character : value)
        {
            if (characters.find(character) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }
}

int main()
{
    const std::string generated =
        password::generate(password::DEFAULT_GENERATED_LENGTH);
    assert(generated.size() == password::DEFAULT_GENERATED_LENGTH);
    assert(hasCharacterFrom(generated, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
    assert(hasCharacterFrom(generated, "abcdefghijklmnopqrstuvwxyz"));
    assert(hasCharacterFrom(generated, "0123456789"));
    assert(hasCharacterFrom(generated, "!@#$%^&*()-_=+[]{}:,.?"));

    const std::string custom = password::generate(32);
    assert(custom.size() == 32);
    assert(password::generate(32) != custom);

    bool tooShortRejected = false;
    try
    {
        static_cast<void>(password::generate(password::MIN_GENERATED_LENGTH - 1));
    }
    catch (const std::runtime_error &)
    {
        tooShortRejected = true;
    }
    assert(tooShortRejected);

    bool tooLongRejected = false;
    try
    {
        static_cast<void>(password::generate(password::MAX_GENERATED_LENGTH + 1));
    }
    catch (const std::runtime_error &)
    {
        tooLongRejected = true;
    }
    assert(tooLongRejected);

    std::cout << "Password generation tests passed.\n";
    return 0;
}
