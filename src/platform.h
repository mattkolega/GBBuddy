#pragma once

#include <expected>
#include <filesystem>
#include <optional>

enum class DialogError {
    Failed,
    Cancelled
};

struct FileFilter {
    const char* name;
    const char* extensions; // Comma-separated string
};

namespace platform {
    // Grabs the path to the app's location.
    // If on macOS and app is bundled in .app, the resource directory will be returned instead.
    // Avoid writing files to this path.
    [[nodiscard]]
    std::optional<std::filesystem::path> getBasePath();

    // Grabs the path to the app's data path.
    // E.g. on Windows: %APPDATA%/Roaming/org/app
    [[nodiscard]]
    std::optional<std::filesystem::path> getDataPath();

    // Displays a file dialog to grab an individual file.
    // Filters format should as follows `{{"Images", "png,jpeg"}}`.
    [[nodiscard]]
    std::expected<std::filesystem::path, DialogError> openFileDialog(std::initializer_list<FileFilter> filters = {});
}
