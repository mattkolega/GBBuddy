#pragma once

#include <string>
#include <vector>

namespace Dialog {
    std::string openFile(const char *dialogTitle, std::vector<const char *> filters, const char *filterDescription);
};