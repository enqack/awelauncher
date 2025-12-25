#include "LauncherController.h"
#include "../models/LauncherModel.h"
#include "../utils/MRUTracker.h"
#include <QDebug>

LauncherController::LauncherController(QObject *parent)
    : QObject(parent)
{
}

#include <QProcess>
#include <QStandardPaths>
#include <QCoreApplication>
#include <cstdlib>

void LauncherController::setModel(LauncherModel* model)
{
    m_model = model;
}

void LauncherController::filter(const QString &text)
{
    if (m_model) {
        m_model->filter(text);
    }
}

void LauncherController::activate(int index)
{
    qDebug() << "Activate requested for index:" << index;
    if (!m_model) return;

    QModelIndex modelIndex = m_model->index(index, 0);
    if (!modelIndex.isValid()) return;

    // Use roles to get data directly from model
    bool isTerminal = m_model->data(modelIndex, LauncherModel::TerminalRole).toBool();
    QString exec = m_model->data(modelIndex, LauncherModel::ExecRole).toString(); 
    
    if (exec.isEmpty()) return;

    QString program;
    QStringList args;

    if (isTerminal) {
        // Simple terminal detection strategy
        QString term;
        // Check standard env var or common terminals
        const char *termEnv = std::getenv("TERM");
        if (termEnv) {
             // Does not help much, usually "xterm" or "linux". 
             // We need the emulator binary.
        }

        const QStringList terminals = {
            "gnome-terminal", "konsole", "alacritty", "kitty", "xfce4-terminal", "urxvt", "xterm", "weston-terminal" 
        };

        for (const auto &t : terminals) {
            if (!QStandardPaths::findExecutable(t).isEmpty()) {
                term = t;
                break;
            }
        }
        
        // Fallback
        if (term.isEmpty()) term = "x-terminal-emulator";

        // Logic for buffering the command
        // Simple "-e" works for most (xterm, alacritty, kitty legacy, konsole -e)
        // gnome-terminal uses "--" or "-e" depending on version, let's try standard "-e" or "--"
        
        program = term;
        // Most terminals accept: <term> -e <command>
        args << "-e" << exec;
        
        // Split exec into args? Ideally yes, but -e usually takes the rest string or single arg.
        // For simple MVP let's pass the string.
        // Note: QProcess arguments need to be separate strings. 
        // If exec is "htop", args are "-e", "htop".
        // If exec is "vim /tmp/foo", args are "-e", "vim /tmp/foo" (depending on terminal)
        // Let's assume the exec string is a single command for now or simple shell string.
        
        // Refined: split exec string by spaces for QProcess?
        // Actually, startDetached(program, args) is safer.
        // But extracting args from an Exec string like "vim %f" (cleaned to "vim") is hard.
        // Let's rely on QProcess::startDetached(command) override that parses shell command?
        // Qt6: startDetached(program, args) is preferred.
        
        // Simplified approach: just run it. 
        // If terminal is alacritty: alacritty -e htop
    } else {
        // Direct launch
        // We use split to separate program from args
        QStringList parts = exec.split(' ');
        if (!parts.isEmpty()) {
            program = parts.takeFirst();
            args = parts;
        }
    }

    if (!program.isEmpty()) {
        // Record MRU
        QString itemId = m_model->data(modelIndex, LauncherModel::IdRole).toString();
        if (!itemId.isEmpty()) {
            MRUTracker::instance().recordActivation(itemId);
        }
        
        qDebug() << "Launching:" << program << args;
        QProcess::startDetached(program, args);
        // Close launcher (optional, maybe configurable)
        QCoreApplication::quit(); 
        // Or if not quitting, at least hide/reset. 
        // For now, let's quit as typical for launchers.
    }
}
