#pragma once

#include <QString>
#include <QStringList>

class TerminalUtils {
public:
    static QString findTerminal();
    static QStringList wrapCommand(const QString& terminalBin, const QString& command, bool holdOpen);
};
