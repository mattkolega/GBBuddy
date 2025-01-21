#include "dialog.h"

#include <tinyfiledialogs.h>

namespace Dialog {
    std::string openFile(const char *dialogTitle, std::vector<const char *> filters, const char *filterDescription) {
        auto filename = tinyfd_openFileDialog(
            dialogTitle,
            nullptr,
            filters.size(),
            filters.data(),
            filterDescription,
            0
        );
        return std::string(filename);
    }
}