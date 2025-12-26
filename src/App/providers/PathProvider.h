#pragma once

#include <vector>
#include "../models/LauncherModel.h"

/**
 * @class PathProvider
 * @brief Scans system PATH for executables.
 */
class PathProvider {
public:
    static std::vector<LauncherItem> scan();
};
