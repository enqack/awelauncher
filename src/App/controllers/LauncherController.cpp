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
    
    // Window mode: activate window
    if (m_windowProvider && exec.isEmpty()) {
        if (!itemId.isEmpty()) {
            m_windowProvider->activateWindow(itemId);
            qDebug() << "Activated window:" << itemId;
        }
        QCoreApplication::quit();
        return;
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
