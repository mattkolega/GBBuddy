#pragma once

#include <filesystem>
#include <optional>
#include <vector>

#include "types.h"

namespace fs {
    // Opens a file in binary mode and loads its contents into a vector of bytes
    std::optional<std::vector<u8>> loadFileIntoBuffer(const std::filesystem::path& filepath);
}
