#include "SSHProvider.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QDebug>
#include <QProcessEnvironment>

std::vector<LauncherItem> SSHProvider::scan(const QString& terminalCmd, bool parseKnownHosts) {
    std::vector<LauncherItem> items;
    QStringList seenHosts;
    
    auto addHost = [&](const QString& host, const QString& alias, const QString& user) {
        if (seenHosts.contains(host) && alias.isEmpty()) return; // Dedup
        seenHosts.append(host);
        
        LauncherItem item;
        item.id = "ssh:" + host;
        
        if (!alias.isEmpty()) {
            item.primary = alias;
            item.secondary = "SSH -> " + host + (user.isEmpty() ? "" : " ("+user+")");
        } else {
            item.primary = host;
            item.secondary = "SSH Host" + (user.isEmpty() ? "" : " ("+user+")");
        }
        
        item.iconKey = "utilities-terminal";
        
        // Determine Terminal Command
        // Priority: Passed cmd (config/xdg) > $TERM
        QString term = terminalCmd;
        if (term.isEmpty()) {
             term = QProcessEnvironment::systemEnvironment().value("TERM", "xterm");
        }
        
        // Command execution structure: 
        // We usually need "term -e ssh host" or similar.
        // xdg-terminal-exec takes the command as args directly: "xdg-terminal-exec ssh host"
        // But if it's a raw terminal like "foot", it often needs "-e".
        // Let's assume the user config provides the full prefix e.g. "foot -e".
        // If config is EMPTY, we use xdg-terminal-exec if available?
        // For now, let's construct a "blind" command:
        // "ssh host" - wait, LauncherController executes via QProcess::startDetached.
        // It treats `exec` as the program.
        // So we need to put the full command line in `exec`.
        
        // If using default logic:
        // item.exec = "xdg-terminal-exec ssh " + host;
        
        // Let's refine the logic in Controller or here? 
        // Ideally here.
        QString sshCmd = "ssh " + host;
        if (!term.isEmpty()) {
            item.exec = term + " " + sshCmd;
        } else {
            // Fallback to xdg-terminal-exec default
             item.exec = "xdg-terminal-exec " + sshCmd;
        }
        
        item.terminal = false; // We are launching the terminal wrapper ourselves, so we don't need the runner to wrap it again.
        items.push_back(item);
    };
    
    // 1. Parse Config
    QFile configFile(QDir::homePath() + "/.ssh/config");
    if (configFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&configFile);
        QString currentHost;
        QString currentUser;
        QString line;
        while (in.readLineInto(&line)) {
            line = line.trimmed();
            if (line.startsWith("Host ", Qt::CaseInsensitive)) {
                // New block
                QString hosts = line.mid(5).trimmed();
                // "Host *" is generic, ignore
                if (hosts == "*") continue;
                
                QStringList aliases = hosts.split(" ", Qt::SkipEmptyParts);
                for (const auto& a : aliases) {
                    addHost(a, a, ""); // User unknown yet
                }
            }
        }
        configFile.close();
    }
    
    // 2. Parse Known Hosts (hashed hosts are skipped)
    if (parseKnownHosts) {
        QFile kh(QDir::homePath() + "/.ssh/known_hosts");
        if (kh.open(QIODevice::ReadOnly)) {
             QTextStream in(&kh);
             QString line;
             while(in.readLineInto(&line)) {
                 if (line.trimmed().isEmpty()) continue;
                 QString hostSection = line.split(" ").first();
                 if (hostSection.startsWith("|1|")) continue; // Hashed
                 
                 QStringList names = hostSection.split(",");
                 for (const auto& name : names) {
                     if (!name.startsWith("[") && !name.contains("*")) { // clean IP or hostname
                         addHost(name, "", "");
                     }
                 }
             }
        }
    }
    
    return items;
}
