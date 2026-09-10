#pragma once

#include <string>

namespace clipboard {

void copy(
    const std::string& text
);

void clear();

bool contains(
    const std::string& text
);

}