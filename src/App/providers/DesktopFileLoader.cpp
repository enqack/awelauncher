#include "DesktopFileLoader.h"
#include <QStandardPaths>
#include <QDir>
#include <QDirIterator>
#include <QSettings>
#include <QRegularExpression>
#include <QDebug>

std::vector<LauncherItem> DesktopFileLoader::scan()
{
    std::vector<LauncherItem> items;
    QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    
    // Add some common fallbacks if standard paths are empty (unlikely on Linux)
    if (dirs.isEmpty()) {
        dirs << "/usr/share/applications" << QDir::homePath() + "/.local/share/applications";
    }
    
    qDebug() << "Scanning for desktop files in:" << dirs;

    // Keep track of IDs to avoid duplicates (e.g. user override)
    QHash<QString, bool> seenIds;

    for (const auto &path : dirs) {
        QDirIterator it(path, QStringList() << "*.desktop", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString filePath = it.next();
            QString id = it.fileInfo().fileName();

            if (seenIds.contains(id)) continue;
            seenIds.insert(id, true);

            QSettings desktopFile(filePath, QSettings::IniFormat);
            
            desktopFile.beginGroup("Desktop Entry");
            
            if (desktopFile.value("NoDisplay", false).toBool() || desktopFile.value("Hidden", false).toBool()) {
                continue;
            }

            // XDG: TryExec - if binary missing, ignore app
            QString tryExec = desktopFile.value("TryExec").toString();
            if (!tryExec.isEmpty()) {
                // If absolute path, check existence
                if (tryExec.startsWith("/")) {
                   if (!QFile::exists(tryExec)) continue;
                } else {
                    // Check path
                    if (QStandardPaths::findExecutable(tryExec).isEmpty()) continue;
                }
            }
            
            QString name = desktopFile.value("Name").toString();
            QString exec = desktopFile.value("Exec").toString();
            QString icon = desktopFile.value("Icon").toString();
            QString comment = desktopFile.value("Comment").toString();
            QString type = desktopFile.value("Type").toString();
            bool terminal = desktopFile.value("Terminal", false).toBool();

            if (type != "Application" || name.isEmpty() || exec.isEmpty()) {
                continue;
            }
            
            // Clean up Exec (XDG codes)
            // Remove %f, %F, %u, %U, %i, %c, %k
            // Naive approach: remove " %<char>"
            exec.remove(QRegularExpression(" %[%a-zA-Z]"));

            items.push_back({
                id,
                name,
                comment.isEmpty() ? exec : comment,
                exec,
                icon.isEmpty() ? "application-x-executable" : icon,
                false,
                terminal
            });
            
            desktopFile.endGroup();
        }
    }
    
    qDebug() << "Found" << items.size() << "desktop applications";
    
    // Sort by name
    std::sort(items.begin(), items.end(), [](const LauncherItem& a, const LauncherItem& b) {
        return a.primary.compare(b.primary, Qt::CaseInsensitive) < 0;
    });

    return items;
}
