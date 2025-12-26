#pragma once

#include <vector>
#include "../models/LauncherModel.h"

class SSHProvider {
public:
    static std::vector<LauncherItem> scan(const QString& terminalCmd, bool parseKnownHosts);
};
