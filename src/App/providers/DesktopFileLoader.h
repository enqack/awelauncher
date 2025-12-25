#pragma once

#include <vector>
#include "../models/LauncherModel.h"

class DesktopFileLoader
{
public:
    static std::vector<LauncherItem> scan();
};
