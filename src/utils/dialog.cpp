#include "dialog.h"

#include <tinyfiledialogs.h>

namespace Dialog {
    std::string openFile(std::string_view dialogTitle, std::vector<const char *> filters, std::string_view filterDescription) {
        auto filename = tinyfd_openFileDialog(
            std::string(dialogTitle).c_str(),
            nullptr,
            filters.size(),
            filters.data(),
            std::string(filterDescription).c_str(),
            0
        );
        return std::string(filename);
    }
}