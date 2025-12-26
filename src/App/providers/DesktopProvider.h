#pragma once

#include <vector>
#include "../models/LauncherModel.h"
#include "DesktopFileLoader.h"

/**
 * @class DesktopProvider
 * @brief Wrapper for DesktopFileLoader to conform to provider pattern.
 */
class DesktopProvider {
public:
    static std::vector<LauncherItem> scan();
};
