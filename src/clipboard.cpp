#include "clipboard.hpp"

#include <cstdio>
#include <stdexcept>
#include <string>

namespace clipboard {

void copy(
    const std::string& text
)
{
    FILE* pipe = popen("pbcopy", "w");

    if (pipe == nullptr) {
        throw std::runtime_error(
            "Could not open clipboard"
        );
    }

    const std::size_t written =
        fwrite(
            text.data(),
            sizeof(char),
            text.size(),
            pipe
        );

    if (written != text.size()) {
        pclose(pipe);

        throw std::runtime_error(
            "Failed to write to clipboard"
        );
    }

    const int status =
        pclose(pipe);

    if (status != 0) {
        throw std::runtime_error(
            "pbcopy failed"
        );
    }
}

void clear()
{
    FILE* pipe = popen(
        "pbcopy",
        "w"
    );

    if (pipe == nullptr) {
        throw std::runtime_error(
            "Could not open clipboard"
        );
    }

    const int status =
        pclose(pipe);

    if (status != 0) {
        throw std::runtime_error(
            "Failed to clear clipboard"
        );
    }
}

bool contains(
    const std::string& text
)
{
    FILE* pipe = popen(
        "pbpaste",
        "r"
    );

    if (pipe == nullptr) {
        throw std::runtime_error(
            "Could not read clipboard"
        );
    }

    std::string clipboardText;

    char buffer[256];

    while (true) {
        const std::size_t count =
            fread(
                buffer,
                sizeof(char),
                sizeof(buffer),
                pipe
            );

        if (count == 0) {
            break;
        }

        clipboardText.append(
            buffer,
            count
        );
    }

    const int status =
        pclose(pipe);

    if (status != 0) {
        throw std::runtime_error(
            "pbpaste failed"
        );
    }

    return clipboardText == text;
}

}