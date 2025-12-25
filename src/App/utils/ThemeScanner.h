#pragma once

class Theme;

/**
 * @class ThemeScanner
 * @brief Helper class to scan external configuration files for theme colors.
 */
class ThemeScanner
{
public:
    static bool tryLoadFromBase16File(Theme* theme);
    static bool tryLoadFromKitty(Theme* theme);
    static bool tryLoadFromAlacritty(Theme* theme);
    static bool tryLoadFromWezTerm(Theme* theme);
};
