#pragma once

#include <vector>
#include <QString>
#include "../models/LauncherModel.h"

/**
 * @class ProcessProvider
 * @brief Handles reading system processes for 'top' and 'kill' modes.
 */
class ProcessProvider {
public:
    enum SortMode { CPU, MEMORY };

    /** @brief Returns list of processes, optionally sorted by usage. */
    static std::vector<LauncherItem> scan(bool topMode, int limit, SortMode sort, bool showSystem);
    
    /** @brief Helper to kill a process. */
    static bool killProcess(int pid, int signal);
};
