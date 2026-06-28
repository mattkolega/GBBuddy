#include "fs.h"

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <optional>
#include <vector>

#include "log.h"
#include "types.h"

std::optional<std::vector<u8>> fs::loadFileIntoBuffer(const std::filesystem::path &filepath) {
    std::ifstream file {filepath, std::ios::binary};
    if (!file) {
        log::fatal("Failed to open file: `{}` Error: {}", filepath, std::strerror(errno));
        return std::nullopt;
    }

    // Get filesize
    file.seekg(0, std::ios::end);
    auto fileSize = file.tellg();
    if (fileSize < 0) {
        log::fatal("Failed to open file: `{}` Error: Invalid file size", filepath);
        return std::nullopt;
    }
    file.seekg(0, std::ios::beg);

    std::vector<u8> buffer(fileSize);

    // Copy file data to buffer
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    if (!file) {
        log::fatal("Failed to open file: `{}` Error: {}", filepath, std::strerror(errno));
        return std::nullopt;
    }

    return buffer;
}
