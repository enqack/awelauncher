#include "ProcessProvider.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <algorithm>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

struct ProcessInfo {
    int pid;
    QString name;
    QString cmdline;
    long unsigned int utime = 0;
    long unsigned int stime = 0;
    long long rss = 0;
    double cpuUsage = 0.0;
};

// Helper: Get system uptime/clk_tck for CPU calc
static long getSysUptime() {
    QFile f("/proc/uptime");
    if (f.open(QIODevice::ReadOnly)) {
        QString s = f.readAll();
        return s.split(" ").first().toDouble();
    }
    return 0;
}

static long Hertz = sysconf(_SC_CLK_TCK);

// Helper: Read a single process from /proc/[pid]
static bool readProc(const QString& pidStr, ProcessInfo& out) {
    bool ok;
    int pid = pidStr.toInt(&ok);
    if (!ok) return false;
    out.pid = pid;

    // Read cmdline
    QFile fCmd("/proc/" + pidStr + "/cmdline");
    if (fCmd.open(QIODevice::ReadOnly)) {
        QByteArray data = fCmd.readAll();
        // cmdline is null-delimited, replace with spaces for display
        out.cmdline = QString::fromUtf8(data).replace('\0', ' ').trimmed();
        fCmd.close();
    }

    // Read stat for CPU/Mem/Name
    QFile fStat("/proc/" + pidStr + "/stat");
    if (fStat.open(QIODevice::ReadOnly)) {
        QString content = fStat.readAll();
        // Format of /proc/[pid]/stat is tricky because comm can contain spaces/parens.
        // Usually: pid (comm) state ppid ...
        int firstParen = content.indexOf('(');
        int lastParen = content.lastIndexOf(')');
        if (firstParen != -1 && lastParen != -1) {
            out.name = content.mid(firstParen + 1, lastParen - firstParen - 1);
            
            // The rest after lastParen
            QString rest = content.mid(lastParen + 2);
            QStringList parts = rest.split(' ', Qt::SkipEmptyParts);
            // parts[0] is state.
            // utime is parts[11], stime is parts[12] (0-indexed relative to rest)
            // But 'rest' starts at field 3 (state maps to 3 in standard man proc).
            // Actually, man 5 proc:
            // (1) pid, (2) comm, (3) state, ...
            // So after ')', the next item is field 3 (state).
            // utime is field 14.
            // stime is field 15.
            // rss is field 24.
            
            // Indices in 'parts' (which starts at field 3):
            // Field 14 corresponds to index 14 - 3 = 11.
            // Field 15 corresponds to index 15 - 3 = 12.
            // Field 24 corresponds to index 24 - 3 = 21.
            
            if (parts.size() >= 22) {
                out.utime = parts[11].toULong();
                out.stime = parts[12].toULong();
                out.rss = parts[21].toLongLong();
            }
        }
    }
    
    if (out.name.isEmpty() && out.cmdline.isEmpty()) return false;
    if (out.name.isEmpty()) out.name = out.cmdline.split(' ').first();
    
    return true;
}

std::vector<LauncherItem> ProcessProvider::scan(bool topMode, int limit, SortMode sort, bool showSystem) {
    std::vector<LauncherItem> items;
    std::vector<ProcessInfo> procs;
    
    QDir procDir("/proc");
    QStringList entries = procDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    
    int myUid = getuid();

    for (const QString& pidStr : entries) {
        // Must be numeric
        bool isNum;
        pidStr.toInt(&isNum);
        if (!isNum) continue;

        // Check owner first if not showSystem
        if (!showSystem) {
             QFileInfo info("/proc/" + pidStr);
             if (info.ownerId() != (uint)myUid) continue;
        }

        ProcessInfo info;
        if (readProc(pidStr, info)) {
            // Calculate pseudo CPU usage (Process Utime+Stime / Uptime) - wait, that's avg over life.
            // For a *snapshot* "top", we really need Delta. 
            // Since we can't wait for a delta, we'll use "Avg CPU over lifetime" or just raw Memory?
            // "Top" usually shows current usage.
            // Without a delta (stateful), showing CPU is misleading for long running processes.
            // BUT, `ps aux` shows %CPU. How? "cpu time / elapsed time".
            // Let's stick to Memory for now as the default robust sort, or just lifetime CPU intensity.
            // Or... RFC said "Snapshot on open".
            // Let's calculate Lifetime Usage = (utime+stime) / Hertz / (ProcessStartTime - SystemBootTime).
            // Accessing starttime is complex (field 22). 
            // Let's assume RSS for sorting if CPU is too hard for a quick snapshot.
            // Actually config lets user sort.
            
            // Lets just store raw values and sort.
            procs.push_back(info);
        }
    }

    // Sort
    if (sort == MEMORY) {
        std::sort(procs.begin(), procs.end(), [](const ProcessInfo& a, const ProcessInfo& b){
            return a.rss > b.rss;
        });
    } else {
        // CPU (using raw ticks as proxy for "hogs over time" since we lack delta)
        std::sort(procs.begin(), procs.end(), [](const ProcessInfo& a, const ProcessInfo& b){
             return (a.utime + a.stime) > (b.utime + b.stime);
        });
    }
    
    // Apply limit for Top mode
    if (topMode && limit > 0 && procs.size() > (size_t)limit) {
        procs.resize(limit);
    }

    // Convert to LauncherItems
    for (const auto& p : procs) {
        LauncherItem item;
        item.id = "proc:" + QString::number(p.pid);
        item.primary = p.name;
        // Format details: PID - RES: X MB
        double rssMb = (p.rss * 4096) / 1024.0 / 1024.0; // RSS is usually pages (4kb)
        item.secondary = QString("PID: %1 | MEM: %2 MB | %3").arg(p.pid).arg(rssMb, 0, 'f', 1).arg(p.cmdline);
        item.iconKey = "application-x-executable"; // Could try to map p.name to desktop icon?
        
        // exec is empty? Or "kill" command?
        // Controller will need to know this is a "Process" to perform the Kill action.
        // We can hack `exec` to store the PID for the controller to read.
        item.exec = "kill:" + QString::number(p.pid); 
        item.terminal = false;
        
        items.push_back(item);
    }
    
    return items;
}

bool ProcessProvider::killProcess(int pid, int signal) {
    return ::kill(pid, signal) == 0;
}
