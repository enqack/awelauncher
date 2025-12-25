#include "LauncherController.h"
#include "../models/LauncherModel.h"
#include "../utils/MRUTracker.h"
#include "../providers/WindowProvider.h"
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

void LauncherController::setWindowProvider(WindowProvider* provider)
{
    m_windowProvider = provider;
}

void LauncherController::setDmenuMode(bool enabled)
{
    m_dmenuMode = enabled;
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

    QString itemId = m_model->data(modelIndex, LauncherModel::IdRole).toString();
    QString exec = m_model->data(modelIndex, LauncherModel::ExecRole).toString();
    QString primary = m_model->data(modelIndex, LauncherModel::PrimaryRole).toString();
    
    // Dmenu mode: print and quit
    if (m_dmenuMode) {
        printf("%s\n", primary.toStdString().c_str());
        fflush(stdout);
        QCoreApplication::quit();
        return;
    }

    
    // Window mode: activate window or move to monitor
    if (m_windowProvider && exec.isEmpty()) {
        if (m_selectionMode == Normal) {
            // If it's a window and we want to move it (could be triggered by a specific hotkey later,
            // but for now let's make it part of the 'window' mode flow if we decide to extend it.
            // Actually, the user wants it *after* the app is chosen.
            
            // To keep it simple: if in window mode, we activate and quit UNLESS we are in a special 'move' state.
            // Let's implement a 'Secondary Action' trigger.
            // For now, let's just make 'window' mode always go to monitor select for testing or if configured.
            // Use case: "Display it after the app is chosen".
            
            m_pendingHandle = itemId;
            m_selectionMode = MonitorSelect;
            m_promptOverride = "Move to Monitor...";
            emit selectionModeChanged();
            emit promptOverrideChanged();
            emit clearSearch();

            // Populate model with monitors
            QStringList outputs = m_windowProvider->getOutputNames();
            std::vector<LauncherItem> monitorItems;
            for (const QString& out : outputs) {
                LauncherItem item;
                item.id = out;
                item.primary = out;
                item.secondary = "Output Monitor";
                item.iconKey = "video-display";
                monitorItems.push_back(item);
            }
            m_model->setItems(monitorItems);
            return;
        } else if (m_selectionMode == MonitorSelect) {
            if (!m_pendingHandle.isEmpty()) {
                m_windowProvider->moveToOutput(m_pendingHandle, primary);
                qDebug() << "Moved window" << m_pendingHandle << "to" << primary;
            }
            QCoreApplication::quit();
            return;
        }
    }
    
    // App mode: launch application
    if (exec.isEmpty()) return;
    
    bool isTerminal = m_model->data(modelIndex, LauncherModel::TerminalRole).toBool();
    QString program;
    QStringList args;

    if (isTerminal) {
        // Find available terminal
        QString term;
        const QStringList terminals = {
            "gnome-terminal", "konsole", "alacritty", "kitty", "xfce4-terminal", "urxvt", "xterm", "weston-terminal" 
        };

        for (const auto &t : terminals) {
            if (!QStandardPaths::findExecutable(t).isEmpty()) {
                term = t;
                break;
            }
        }
        
        if (term.isEmpty()) term = "x-terminal-emulator";
        program = term;
        args << "-e" << exec;
    } else {
        // Direct launch
        QStringList parts = exec.split(' ');
        if (!parts.isEmpty()) {
            program = parts.takeFirst();
            args = parts;
        }
    }

    if (!program.isEmpty()) {
        // Record MRU
        if (!itemId.isEmpty()) {
            MRUTracker::instance().recordActivation(itemId);
        }
        
        qDebug() << "Launching:" << program << args;
        QProcess::startDetached(program, args);
        QCoreApplication::quit();
    }
}

// Window mode actions
void LauncherController::closeWindow(int index)
{
    if (!m_model || !m_windowProvider) return;
    QModelIndex modelIndex = m_model->index(index, 0);
    if (!modelIndex.isValid()) return;
    
    QString windowId = m_model->data(modelIndex, LauncherModel::IdRole).toString();
    if (!windowId.isEmpty()) {
        m_windowProvider->closeWindow(windowId);
        qDebug() << "Closed window:" << windowId;
    }
}

void LauncherController::toggleFullscreen(int index)
{
    if (!m_model || !m_windowProvider) return;
    QModelIndex modelIndex = m_model->index(index, 0);
    if (!modelIndex.isValid()) return;
    
    QString windowId = m_model->data(modelIndex, LauncherModel::IdRole).toString();
    if (!windowId.isEmpty()) {
        m_windowProvider->toggleFullscreen(windowId);
        qDebug() << "Toggled fullscreen:" << windowId;
    }
}

void LauncherController::toggleMaximize(int index)
{
    if (!m_model || !m_windowProvider) return;
    QModelIndex modelIndex = m_model->index(index, 0);
    if (!modelIndex.isValid()) return;
    
    QString windowId = m_model->data(modelIndex, LauncherModel::IdRole).toString();
    if (!windowId.isEmpty()) {
        m_windowProvider->toggleMaximize(windowId);
        qDebug() << "Toggled maximize:" << windowId;
    }
}

void LauncherController::toggleMinimize(int index)
{
    if (!m_model || !m_windowProvider) return;
    QModelIndex modelIndex = m_model->index(index, 0);
    if (!modelIndex.isValid()) return;
    
    QString windowId = m_model->data(modelIndex, LauncherModel::IdRole).toString();
    if (!windowId.isEmpty()) {
        m_windowProvider->toggleMinimize(windowId);
        qDebug() << "Toggled minimize:" << windowId;
    }
}

// Output management
QStringList LauncherController::getOutputs()
{
    if (m_windowProvider) {
        return m_windowProvider->getOutputNames();
    }
    return QStringList();
}

void LauncherController::moveWindowToOutput(int index, const QString& outputName)
{
    if (!m_model || !m_windowProvider) return;
    QModelIndex modelIndex = m_model->index(index, 0);
    if (!modelIndex.isValid()) return;
    
    QString windowId = m_model->data(modelIndex, LauncherModel::IdRole).toString();
    if (!windowId.isEmpty()) {
        m_windowProvider->moveToOutput(windowId, outputName);
        qDebug() << "Moving window to output:" << outputName;
    }
}
