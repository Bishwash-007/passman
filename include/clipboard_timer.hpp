#pragma once

#include <string>

namespace clipboard_timer
{

    void clearAfter(
        const std::string &text,
        unsigned int seconds);

    void cancel();

}