#include "PathProvider.h"
#include <QDir>
#include <QStringList>
#include <QFileInfoList>
#include <QDebug>

std::vector<LauncherItem> PathProvider::scan() {
    std::vector<LauncherItem> items;
    QStringList pathDirs = QString(qgetenv("PATH")).split(":", Qt::SkipEmptyParts);
    
    for (const QString& dir : pathDirs) {
        QDir d(dir);
        if (!d.exists()) continue;
        
        QFileInfoList files = d.entryInfoList(QDir::Files | QDir::Executable, QDir::Name);
        for (const QFileInfo& file : files) {
            LauncherItem item;
            item.id = "path:" + file.fileName();
            // In a real scenario we might want full path, but "primary" is usually display name
            item.primary = file.fileName();
            item.secondary = file.absoluteFilePath();
            item.iconKey = "application-x-executable";
            item.exec = file.fileName(); 
            item.terminal = false;
            item.selected = false;
            items.push_back(item);
        }
    }
    return items;
}
