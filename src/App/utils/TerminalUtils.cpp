#include "TerminalUtils.h"
#include <QStandardPaths>
#include <QDebug>

QString TerminalUtils::findTerminal()
{
    const QStringList terminals = {
        "xdg-terminal-exec", "wezterm", "gnome-terminal", "konsole", "alacritty", "kitty", "xfce4-terminal", "urxvt", "xterm", "weston-terminal", "foot"
    };

    for (const auto &t : terminals) {
        if (!QStandardPaths::findExecutable(t).isEmpty()) {
            return t;
        }
    }
    return "x-terminal-emulator";
}

QStringList TerminalUtils::wrapCommand(const QString& terminalBin, const QString& command, bool holdOpen)
{
    QStringList args;
    
    // Most terminals support -e
    // Some (like xdg-terminal-exec) might just take the command as args
    // But our logic in Controller assumed -e for everything found in the list.
    // If wrapping for hold, we need sh -c trick.
    
    QString shellCmd;
    if (holdOpen) {
         // Wrap for holding: sh -c "cmd; read"
         QString safeExec = command;
         safeExec.replace("\"", "\\\""); 
         shellCmd = QString("%1; echo; echo 'Press Enter to close...'; read").arg(safeExec);
    } else {
         shellCmd = command;
    }

    if (terminalBin.endsWith("xdg-terminal-exec")) {
        // xdg-terminal-exec interprets arguments as command + args, not a shell string.
        // pass sh -c to ensure we can run complex command strings or our hold script
        args << "sh" << "-c" << shellCmd;
    } else if (terminalBin.endsWith("wezterm")) {
        // wezterm prefers "start -- prog args"
        args << "start" << "--" << "sh" << "-c" << shellCmd;
    } else {
        // Legacy/Standard behavior for xterm, konsole, etc.
        // most accept -e
        // if holdOpen, we must use sh -c because shellCmd is a complex script
        if (holdOpen) {
             args << "-e" << "sh" << "-c" << shellCmd;
        } else {
             // For simple launch, we pass command directly with -e
             // This assumes terminals handle 'term -e "full command"' correctly 
             // (splitting or passing to shell internally)
             args << "-e" << command;
        }
    }
    
    return args;
}
