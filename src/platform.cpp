#include "platform.h"

#include <initializer_list>

#include <SDL3/SDL.h>
#include "nfd.hpp"

#include "common/log.h"

namespace platform {
    std::optional<std::filesystem::path> getBasePath() {
        const char* path = SDL_GetBasePath();
        if (!path) {
            log::warn("Failed to get base path. SDL_Error: {}", SDL_GetError());
            return std::nullopt;
        }
        return path;
    }

    std::optional<std::filesystem::path> getDataPath() {
        char* rawPath = SDL_GetPrefPath("mattkolega", "spacebuddy");
        if (!rawPath) {
            log::warn("Failed to get data path. SDL_Error: {}", SDL_GetError());
            return std::nullopt;
        }
        auto path = std::filesystem::path{rawPath};
        SDL_free(rawPath);
        return path;
    }

    std::expected<std::filesystem::path, DialogError> openFileDialog(std::initializer_list<FileFilter> filters) {
        static NFD::Guard nfdGuard;

        NFD::UniquePath outPath;

        // Convert filters to a container which is usable by NFD
        std::vector<nfdfilteritem_t> nfdFilters;
        nfdFilters.reserve(filters.size());
        for (const auto& f : filters) {
            nfdFilters.push_back({f.name, f.extensions});
        }

        nfdresult_t result = NFD::OpenDialog(outPath, nfdFilters.data(), nfdFilters.size());
        if (result == NFD_OKAY) {
            return std::filesystem::path{outPath.get()};
        } else if (result == NFD_CANCEL) {
            log::debug("User closed file dialog.");
            return std::unexpected{DialogError::Cancelled};
        } else {
            log::err("Failed to open file through dialog. NFD Error: {}", NFD::GetError());
            return std::unexpected{DialogError::Failed};
        }
    }
}
