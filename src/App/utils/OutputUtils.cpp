#include "OutputUtils.h"
#include "Config.h"
#include <QGuiApplication>
#include <QCursor>
#include <QDebug>

QScreen* OutputUtils::resolveScreen(const QString& strategy)
{
    const auto screens = QGuiApplication::screens();
    
    if (screens.isEmpty()) {
        return nullptr;
    }

    // fallback
    QScreen* fallback = QGuiApplication::primaryScreen();
    if (!fallback && !screens.isEmpty()) fallback = screens.first();

    if (strategy.isEmpty()) {
        return fallback;
    }

    if (strategy == "follow-mouse") {
        if (QGuiApplication::platformName().contains("wayland")) {
            if (Config::instance().isDebug()) qDebug() << "OutputUtils: 'follow-mouse' disabled on Wayland (no global cursor access) - defaulting to Compositor placement";
            return nullptr;
        }
        
        QPoint cursor = QCursor::pos();
        QScreen* screen = QGuiApplication::screenAt(cursor);
        if (screen) {
            if (Config::instance().isDebug()) qDebug() << "OutputUtils: 'follow-mouse' resolved to" << screen->name() << "at" << cursor;
            return screen;
        } else {
            if (Config::instance().isDebug()) qDebug() << "OutputUtils: 'follow-mouse' failed to find screen at" << cursor << ", returning nullptr (fallback to compositor)";
            return nullptr;
        }
    }

    if (strategy == "follow-focus") {
        // Ideally we'd query the compositor or use a cached "last focused" state if we had one.
        // For now, Qt often treats "primary" as the focus or we just default to primary.
        // TODO: Integrate with WindowProvider if we want true active window tracking.
        qDebug() << "OutputUtils: 'follow-focus' defaulting to primary" << fallback->name();
        return fallback;
    }

    // Try extended searching for name
    for (QScreen* screen : screens) {
        if (screen->name() == strategy) {
             qDebug() << "OutputUtils: Resolved named output" << strategy;
             return screen;
        }
    }

    qWarning() << "OutputUtils: Could not find screen named" << strategy << ", using fallback:" << fallback->name();
    return fallback;
}
