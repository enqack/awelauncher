#include "ThemeScanner.h"
#include "Theme.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

bool ThemeScanner::tryLoadFromBase16File(Theme* theme) {
    QString configDir = QDir::homePath() + "/.base16_theme";
    QFile file(configDir);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    
    qDebug() << "[ThemeScanner] Parsing base16 theme script:" << configDir;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        // Regex for colorXX="hex"
        static QRegularExpression re("color(00|05|0D|01|03|04)=\"([^\"]+)\"");
        auto match = re.match(line);
        if (match.hasMatch()) {
            QString key = match.captured(1);
            QString val = "#" + match.captured(2);
            if (!val.startsWith("#")) val.prepend("#"); 
            
            if (key == "00") theme->m_bg = QColor(val);
            else if (key == "05") theme->m_fg = QColor(val);
            else if (key == "0D") theme->m_accent = QColor(val);
            else if (key == "01") theme->m_selected = QColor(val);
            else if (key == "04") theme->m_muted = QColor(val);
        }
    }
    return true;
}

bool ThemeScanner::tryLoadFromKitty(Theme* theme) {
    QDir dir(QDir::homePath() + "/.config/kitty");
    QStringList filters; filters << "*.conf";
    dir.setNameFilters(filters);
    
    if (!dir.exists()) return false;
    
    QStringList files = dir.entryList(QDir::Files);
    if (files.isEmpty()) return false;
    
    for (const QString& fname : files) {
        QFile file(dir.filePath(fname));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
        
        qDebug() << "[ThemeScanner] Inspecting Kitty config:" << fname;
        QTextStream in(&file);
        bool foundAny = false;
        
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty() || line.startsWith("#")) continue;
            
            static QRegularExpression re("^(foreground|background|color([0-9]+))\\s+(#[0-9a-fA-F]{6})");
            auto match = re.match(line);
            if (match.hasMatch()) {
                QString key = match.captured(1);
                QString val = match.captured(3);
                
                if (key == "background") { theme->m_bg = QColor(val); foundAny = true; }
                else if (key == "foreground") { theme->m_fg = QColor(val); foundAny = true; }
                else if (key == "color4") { theme->m_accent = QColor(val); }
                else if (key == "color0") { theme->m_selected = QColor(val).lighter(120); }
                else if (key == "color8") { theme->m_muted = QColor(val); }
            }
        }
        if (foundAny) return true;
    }
    return false;
}

bool ThemeScanner::tryLoadFromAlacritty(Theme* theme) {
    QDir dir(QDir::homePath() + "/.config/alacritty");
    QStringList filters; filters << "*.toml" << "*.yml";
    dir.setNameFilters(filters);
    
    if (!dir.exists()) return false;
    
    QStringList files = dir.entryList(QDir::Files);
    for (const QString& fname : files) {
        QFile file(dir.filePath(fname));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
        
        qDebug() << "[ThemeScanner] Inspecting Alacritty config:" << fname;
        QTextStream in(&file);
        QString content = in.readAll();
        
        static QRegularExpression reBg("background\\s*=\\s*['\"](#[0-9a-fA-F]{6})['\"]");
        static QRegularExpression reFg("foreground\\s*=\\s*['\"](#[0-9a-fA-F]{6})['\"]");
        
        bool found = false;
        
        auto matchBg = reBg.match(content);
        if (matchBg.hasMatch()) {
             theme->m_bg = QColor(matchBg.captured(1));
             found = true;
        }
        
        auto matchFg = reFg.match(content);
        if (matchFg.hasMatch()) {
             theme->m_fg = QColor(matchFg.captured(1));
        }
        
        static QRegularExpression reBlue("blue\\s*=\\s*['\"](#[0-9a-fA-F]{6})['\"]");
        auto matchBlue = reBlue.match(content);
        if (matchBlue.hasMatch()) theme->m_accent = QColor(matchBlue.captured(1));
        
        static QRegularExpression reBlack("black\\s*=\\s*['\"](#[0-9a-fA-F]{6})['\"]");
        auto matchBlack = reBlack.match(content);
        if (matchBlack.hasMatch()) theme->m_selected = QColor(matchBlack.captured(1)).lighter(120);
        
        if (found) return true;
    }
    return false;
}

bool ThemeScanner::tryLoadFromWezTerm(Theme* theme) {
    QDir dir(QDir::homePath() + "/.config/wezterm/colors");
    dir.setNameFilters({"*.toml"});
    
    if (!dir.exists()) return false;
    
    QStringList files = dir.entryList(QDir::Files);
    for (const QString& fname : files) {
        QFile file(dir.filePath(fname));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
        
        qDebug() << "[ThemeScanner] Inspecting WezTerm theme:" << fname;
        QTextStream in(&file);
        QString content = in.readAll();
        
        static QRegularExpression reBg("background\\s*=\\s*['\"](#[0-9a-fA-F]{6})['\"]");
        static QRegularExpression reFg("foreground\\s*=\\s*['\"](#[0-9a-fA-F]{6})['\"]");
        
        bool found = false;
        
        auto matchBg = reBg.match(content);
        if (matchBg.hasMatch()) {
             theme->m_bg = QColor(matchBg.captured(1));
             found = true;
        }
        
        auto matchFg = reFg.match(content);
        if (matchFg.hasMatch()) theme->m_fg = QColor(matchFg.captured(1));
        
        static QRegularExpression reSelBg("selection_background\\s*=\\s*['\"](#[0-9a-fA-F]{6})['\"]");
        auto matchSel = reSelBg.match(content);
        if (matchSel.hasMatch()) theme->m_selected = QColor(matchSel.captured(1));
        
        static QRegularExpression reCursor("cursor_bg\\s*=\\s*['\"](#[0-9a-fA-F]{6})['\"]");
        auto matchCursor = reCursor.match(content);
        if (matchCursor.hasMatch()) theme->m_accent = QColor(matchCursor.captured(1));
        
        if (found) return true;
    }
    return false;
}
