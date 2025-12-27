#pragma once

#include <QString>
#include <QScreen>

class OutputUtils
{
public:
    /**
     * @brief Resolves the target screen based on a strategy string or output name.
     * 
     * Strategies:
     * - "follow-mouse": Returns the screen containing the cursor.
     * - "follow-focus": Returns the primary screen (limitation of Qt/Wayland focus query without extensive protocol work).
     * - <NAME>: Returns the screen with the matching name.
     * 
     * @param strategy The strategy or output name.
     * @return QScreen* The resolved screen, or primary/first screen as fallback.
     */
    static QScreen* resolveScreen(const QString& strategy);
};
