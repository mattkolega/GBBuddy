#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace Dialog {
    std::string openFile(std::string_view dialogTitle, std::vector<const char *> filters, std::string_view filterDescription);
};