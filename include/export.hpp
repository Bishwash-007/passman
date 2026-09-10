#pragma once

#include <string>
#include <vector>

#include "entry.hpp"

namespace export_format
{
    void write(const std::string &path,
               const std::vector<entry::Entry> &entries);

    std::vector<entry::Entry> read(const std::string &path);
}
