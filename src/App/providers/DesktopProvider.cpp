#include "DesktopProvider.h"

std::vector<LauncherItem> DesktopProvider::scan() {
    // DesktopFileLoader is already implemented as a static scanner
    return DesktopFileLoader::scan();
}
