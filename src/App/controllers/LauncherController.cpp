#include "LauncherController.h"
#include "../models/LauncherModel.h"
#include "../utils/MRUTracker.h"
#include "../providers/WindowProvider.h"
#include <signal.h>
#include "../utils/TerminalUtils.h"
#include "../utils/Constants.h"
#include "../utils/Config.h"
#include "../utils/FilterUtils.h"
#include "../providers/PathProvider.h"
#include "../providers/DesktopProvider.h"
#include "../providers/ProcessProvider.h"
#include "../providers/SSHProvider.h"
#include "../providers/StdinProvider.h"
#include <QFile>
#include <QDir>

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

void LauncherController::setDaemonMode(bool enabled)
{
    m_daemonMode = enabled;
}


void LauncherController::filter(const QString &text)
{
    if (m_model) {
        m_model->filter(text);
    }
}

void LauncherController::activate(int index, int flags)
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
        quit();
        return;
    }

    
    // Window mode: activate window or move to monitor
    if (m_windowProvider && exec.isEmpty()) {
        if (m_selectionMode == Normal) {
            m_windowProvider->activateWindow(itemId);
            hide();
            return;
        } else if (m_selectionMode == MonitorSelect) {
            if (!m_pendingHandle.isEmpty()) {
                m_windowProvider->moveToOutput(m_pendingHandle, primary);
                qDebug() << "Moved window" << m_pendingHandle << "to" << primary;
            }
            hide();
            return;
        }
    }
    
    // App mode: launch application
    if (exec.isEmpty()) return;
    
    // Kill mode: handle "kill:" prefix in exec
    if (exec.startsWith(Constants::ProtocolKill)) {
        QString pidStr = exec.mid(Constants::ProtocolKill.length());
        bool ok;
        int pid = pidStr.toInt(&ok);
        if (ok) {
            // Signal 15 is SIGTERM
            ::kill(pid, 15);
            qDebug() << "Sent SIGTERM to process:" << pid;
            hide();
            return;
        }
    }
    
    bool forceTerm = (flags & ForceTerminal);
    bool holdTerm = (flags & HoldTerminal);
    bool isTerminal = m_model->data(modelIndex, LauncherModel::TerminalRole).toBool() || forceTerm || holdTerm;
    QString program;
    QStringList args;

    if (isTerminal) {
        QString term = TerminalUtils::findTerminal();
        program = term;
        args = TerminalUtils::wrapCommand(term, exec, holdTerm);
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
        hide();
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

void LauncherController::beginMoveToMonitor(int index)
{
    if (!m_model || !m_windowProvider) return;
    QModelIndex modelIndex = m_model->index(index, 0);
    if (!modelIndex.isValid()) return;

    QString itemId = m_model->data(modelIndex, LauncherModel::IdRole).toString();
    QString exec = m_model->data(modelIndex, LauncherModel::ExecRole).toString();

    // Only allow moving windows
    if (!exec.isEmpty()) return;

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
}

void LauncherController::setVisible(bool visible)
{
    if (m_visible == visible) return;
    
    // Lazy UI initialization
    if (visible && m_uiInitializer) {
        m_uiInitializer();
        m_uiInitializer = nullptr;
    }

    m_visible = visible;
    emit windowVisibleChanged(m_visible);
    
    // Clear search when hiding
    if (!visible) {
        emit clearSearch();
    }
}

void LauncherController::hide()
{
    if (m_daemonMode) {
        setVisible(false);
    } else {
        quit();
    }
}

void LauncherController::toggle()
{
    setVisible(!m_visible);
}

void LauncherController::quit()
{
    QCoreApplication::quit();
}

void LauncherController::loadSet(const QString &setName, const QString &modeOverride)
{
    if (!m_model) return;

    Config::ProviderSet activeSet;
    bool usingSet = false;

    if (!setName.isEmpty()) {
        auto configSet = Config::instance().getSet(setName);
        if (configSet) {
            activeSet = *configSet;
            usingSet = true;
            m_mode = setName;
            qDebug() << "LauncherController: Loading Provider Set:" << setName;
        } else {
            qWarning() << "LauncherController: Provider Set not found:" << setName;
        }
    }

    if (!usingSet) {
        QString mode = modeOverride.isEmpty() ? Constants::ProviderDrun : modeOverride;
        activeSet.providers.append(mode);
        activeSet.name = mode;
        m_mode = mode;

        if (activeSet.providers.contains(Constants::ProviderTop)) {
            activeSet.prompt = "Top > ";
            activeSet.icon = "utilities-system-monitor";
        } else if (activeSet.providers.contains(Constants::ProviderKill)) {
            activeSet.prompt = "Kill > ";
            activeSet.icon = "process-stop";
        } else if (activeSet.providers.contains(Constants::ProviderSSH)) {
            activeSet.prompt = "SSH > ";
            activeSet.icon = "network-server";
        }
    }
    emit modeChanged();

    // Notify model of active set (for Pins/Aliases)
    if (m_model) {
        m_model->setSetName(activeSet.name);
    }

    // Update prompt/icon
    m_prompt = activeSet.prompt.isEmpty() ? "Search..." : activeSet.prompt;
    emit promptChanged();

    if (!activeSet.icon.isEmpty()) {
        m_icon = activeSet.icon;
    } else {
        m_icon = "qrc:/awelauncher/assets/logo.png";
    }
    emit iconChanged();

    // Apply layout overrides
    if (usingSet) {
        QMap<QString, QString> setOverrides;
        if (activeSet.layout.width) setOverrides["window.width"] = QString::number(*activeSet.layout.width);
        if (activeSet.layout.height) setOverrides["window.height"] = QString::number(*activeSet.layout.height);
        if (!activeSet.layout.anchor.isEmpty()) setOverrides["window.anchor"] = activeSet.layout.anchor;
        if (activeSet.layout.margin) setOverrides["window.margin"] = QString::number(*activeSet.layout.margin);
        Config::instance().setOverrides(setOverrides);
    }

    // Aggregation
    std::vector<LauncherItem> aggregatedItems;
    for (const QString& providerName : activeSet.providers) {
        if (providerName == Constants::ProviderRun) {
            auto items = PathProvider::scan();
            aggregatedItems.insert(aggregatedItems.end(), items.begin(), items.end());
        } else if (providerName == Constants::ProviderDrun) {
            auto items = DesktopProvider::scan();
            aggregatedItems.insert(aggregatedItems.end(), items.begin(), items.end());
        } else if (providerName == Constants::ProviderTop) {
            int limit = Config::instance().getInt("top.limit", 10);
            QString sortStr = Config::instance().getString("top.sort", "cpu");
            ProcessProvider::SortMode sort = (sortStr == "memory") ? ProcessProvider::MEMORY : ProcessProvider::CPU;
            auto items = ProcessProvider::scan(true, limit, sort, false);
            aggregatedItems.insert(aggregatedItems.end(), items.begin(), items.end());
        } else if (providerName == Constants::ProviderKill) {
            bool showSystem = Config::instance().getString("kill.show_system", "false") == "true";
            auto items = ProcessProvider::scan(false, -1, ProcessProvider::MEMORY, showSystem);
            aggregatedItems.insert(aggregatedItems.end(), items.begin(), items.end());
        } else if (providerName == Constants::ProviderSSH) {
            QString termCmd = Config::instance().getString("ssh.terminal", "");
            bool parseKnown = Config::instance().getString("ssh.parse_known_hosts", "true") == "true";
            auto items = SSHProvider::scan(termCmd, parseKnown);
            aggregatedItems.insert(aggregatedItems.end(), items.begin(), items.end());
        } else if (providerName == Constants::ProviderWindow) {
            if (m_windowProvider) {
                auto windows = m_windowProvider->getWindows();
                std::vector<LauncherItem> wItems(windows.begin(), windows.end());
                aggregatedItems.insert(aggregatedItems.end(), wItems.begin(), wItems.end());
            }
        }
    }

    // Filtering
    if (!activeSet.filter.include.isEmpty() || !activeSet.filter.exclude.isEmpty()) {
        std::vector<LauncherItem> filtered;
        for (const auto& item : aggregatedItems) {
            bool keep = true;
            if (!activeSet.filter.exclude.isEmpty()) {
                if (FilterUtils::matches(item.primary, activeSet.filter.exclude) || FilterUtils::matches(item.id, activeSet.filter.exclude)) keep = false;
            }
            if (keep && !activeSet.filter.include.isEmpty()) {
                bool included = false;
                if (FilterUtils::matches(item.primary, activeSet.filter.include) || FilterUtils::matches(item.id, activeSet.filter.include)) included = true;
                if (!included) keep = false;
            }
            if (keep) filtered.push_back(item);
        }
        aggregatedItems = filtered;
    }

    m_model->setItems(aggregatedItems);
}

#include <QWindow>
void LauncherController::requestFocus()
{
    if (m_mainWindow) {
        if (Config::instance().isDebug()) qDebug() << "LauncherController: Forcefully requesting focus via QWindow::requestActivate()";
        // m_mainWindow->show(); // CRASH FIX: Don't call show() repeatedly, it may reset IME/Surface state
        m_mainWindow->raise();
        m_mainWindow->requestActivate();
    } else {
        qWarning() << "LauncherController: requestFocus called but no QWindow set!";
    }
}
