/**
 * @file main.cpp
 * @brief Entry point for awelauncher.
 * 
 * This file handles the application initialization, command-line argument parsing,
 * and setting up the QML engine with necessary context properties.
 * 
 * @section CLI Command Line Interface
 * awelauncher supports the following command-line options:
 * - @c --show, @c -s : Select provider mode (@c drun, @c run, @c window).
 * - @c --theme, @c -t : Specify the theme name to load.
 * - @c --prompt, @c -p : Custom prompt text.
 * - @c --width : Override window width in pixels.
 * - @c --height : Override window height in pixels.
 * - @c --anchor, @c -a : Set window anchoring (center, top, etc.).
 * - @c --margin, @c -m : Set window margin from anchor point.
 * - @c --dmenu, @c -d : Run in dmenu compatibility mode.
 * - @c --overlay : Force the window to the overlay layer.
 * - @c --debug, @c -g : Enable verbose debug logging.
 * - @c --version : Display version information.
 * - @c --help : Display help message.
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtQml>
#include <QCommandLineParser>
#include <QElapsedTimer>
#include <QSurfaceFormat>
#include "App/controllers/LauncherController.h"
#include "App/models/LauncherModel.h"
#include "App/utils/Theme.h"
#include "App/providers/IconProvider.h"
#include "App/providers/DesktopFileLoader.h"
#include "App/providers/WindowProvider.h"
#include "App/providers/StdinProvider.h"
#include "App/providers/PathProvider.h"
#include "App/providers/DesktopProvider.h"
#include "App/providers/ProcessProvider.h"
#include "App/providers/SSHProvider.h"


#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include "App/utils/Config.h"
#include "App/utils/FilterUtils.h"
#include "App/utils/Constants.h"

// Profiling helper
#define PROFILE_POINT(name) \
    if (debugMode) qDebug() << "[PROFILE]" << name << ":" << timer.elapsed() << "ms";

void personalMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    bool debug = Config::instance().isDebug();
    
    switch (type) {
    case QtDebugMsg:
    case QtInfoMsg:
        if (debug) fprintf(stderr, "%s\n", qPrintable(msg));
        break;
    case QtWarningMsg:
    case QtCriticalMsg:
    case QtFatalMsg:
        fprintf(stderr, "%s\n", qPrintable(msg));
        if (type == QtFatalMsg) abort();
        break;
    }
}

int main(int argc, char *argv[])
{
    // Force Wayland Layer Shell integration
    qputenv("QT_WAYLAND_SHELL_INTEGRATION", "layer-shell");
    
    QGuiApplication app(argc, argv);
    
    QElapsedTimer timer;
    timer.start();

    app.setApplicationName("awelauncher");
    app.setApplicationVersion(APP_VERSION);
    app.setOrganizationName("awelauncher");
    app.setOrganizationDomain("awelauncher");
    app.setDesktopFileName("awelaunch");
    
    // Parse command-line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("Run in awe! - A system engineer’s launcher.");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption showOption(QStringList() << "s" << "show",
        "Show specific provider mode: drun, run, window, top, kill, ssh", "mode", "drun");
    parser.addOption(showOption);
    
    QCommandLineOption themeOption(QStringList() << "t" << "theme",
        "Override theme name", "name");
    parser.addOption(themeOption);
    
    QCommandLineOption promptOption(QStringList() << "p" << "prompt",
        "Set custom prompt text", "text");
    parser.addOption(promptOption);
    
    QCommandLineOption debugOption(QStringList() << "g" << "debug",
        "Enable debug output");
    parser.addOption(debugOption);

    QCommandLineOption clearCacheOption(QStringList() << "c" << "clear-cache",
        "Clear icon cache on startup");
    parser.addOption(clearCacheOption);
    
    // v0.2.0 Overrides
    QCommandLineOption widthOption("width", "Override window width", "px");
    parser.addOption(widthOption);
    
    QCommandLineOption heightOption("height", "Override window height", "px");
    parser.addOption(heightOption);
    
    QCommandLineOption anchorOption({"a", "anchor"}, "Window anchor (center, top, bottom, left, right)", "anchor");
    QCommandLineOption marginOption({"m", "margin"}, "Window anchor margin", "margin");
    parser.addOption(anchorOption);
    parser.addOption(marginOption);
    
    QCommandLineOption dmenuOption(QStringList() << "d" << "dmenu", "Run in dmenu mode (read stdin, print to stdout)");
    parser.addOption(dmenuOption);
    
    // Tier 1: Provider Sets
    QCommandLineOption setOption("set", "Activate a named provider set (e.g. dev, media)", "name");
    parser.addOption(setOption);

    QCommandLineOption overlayOption("overlay", "Force window to use Overlay layer (ignore status bars)");
    parser.addOption(overlayOption);

    
    parser.process(app);
    
    // Start profiling timer
    bool debugMode = parser.isSet(debugOption);
    Config::instance().setDebug(debugMode);
    qInstallMessageHandler(personalMessageHandler);

    if (debugMode) {
        qDebug() << "[Diagnostics] Platform Name:" << app.platformName();
        qDebug() << "[Diagnostics] QT_WAYLAND_SHELL_INTEGRATION:" << qgetenv("QT_WAYLAND_SHELL_INTEGRATION");
    }

    if (parser.isSet(clearCacheOption)) {
        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/awelauncher/icons";
        QDir dir(cacheDir);
        if (dir.exists()) {
            dir.removeRecursively();
            if (debugMode) qDebug() << "Cleared icon cache:" << cacheDir;
        }
    }
    
    PROFILE_POINT("App init");

    // Load Config
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/awelauncher/config.yaml";
    Config::instance().ensureDefaults();
    Config::instance().load(configPath);
    
    // Apply CLI overrides
    QMap<QString, QString> overrides;
    if (parser.isSet(widthOption)) overrides["window.width"] = parser.value(widthOption);
    if (parser.isSet(heightOption)) overrides["window.height"] = parser.value(heightOption);
    if (parser.isSet(anchorOption)) overrides["window.anchor"] = parser.value(anchorOption);
    if (parser.isSet(marginOption)) overrides["window.margin"] = parser.value(marginOption);
    if (parser.isSet(overlayOption)) overrides["window.layer"] = "overlay";
    Config::instance().setOverrides(overrides);
    
    PROFILE_POINT("Config loaded");

    QQmlApplicationEngine engine;

    // Register singletons/providers
    
    // Icon provider
    engine.addImageProvider("icon", new IconProvider());

    // Core Controller
    LauncherController controller;
    engine.rootContext()->setContextProperty("Controller", &controller);

    // Theme (Singleton-like)
    // We use a static instance for the singleton registration lambda
    static Theme theme;
    
    // Load theme from config or CLI override
    QString themeName = parser.value(themeOption);
    if (themeName.isEmpty()) {
        themeName = Config::instance().getString("general.theme", "default");
    }
    if (themeName.isEmpty()) themeName = "default";
    theme.load(themeName);
    
    PROFILE_POINT("Theme loaded");

    qmlRegisterSingletonInstance("awelauncher", 1, 0, "AppTheme", &theme);

    // Model
    LauncherModel model;
    engine.rootContext()->setContextProperty("LauncherModel", &model);
    
    // Expose CLI arguments to QML
    QString showMode = parser.value(showOption);
    QString promptText = parser.value(promptOption);
    if (promptText.isEmpty()) promptText = "Search...";
    
    // Set show mode on model
    model.setShowMode(showMode);
    
    engine.rootContext()->setContextProperty("cliShowMode", showMode);
    engine.rootContext()->setContextProperty("cliPrompt", promptText);
    engine.rootContext()->setContextProperty("debugMode", debugMode);
    
    // Connect logic
    controller.setModel(&model);

    // --- Provider Aggregation Logic ---
    
    Config::ProviderSet activeSet;
    activeSet.name = "adhoc";
    bool usingSet = false;
    
    // 1. Determine which Set to use
    if (parser.isSet(setOption)) {
        QString setName = parser.value(setOption);
        auto configSet = Config::instance().getSet(setName);
        if (configSet) {
            activeSet = *configSet;
            usingSet = true;
            qDebug() << "Loaded Provider Set:" << setName;
        } else {
            qWarning() << "Provider Set not found:" << setName << "- falling back to defaults";
        }
    } 
    
    // Fallback if no set loaded: Check for --show argument (legacy/ad-hoc mode)
    if (!usingSet) {
        QString mode = showMode.isEmpty() ? Constants::ProviderDrun : showMode;
        
        // Construct an ad-hoc set
        activeSet.providers.append(mode);
        activeSet.name = mode;
        
        // Special case: if dmenu flag is on, force dmenu provider
        if (parser.isSet(dmenuOption)) {
            activeSet.providers.clear();
            activeSet.providers.append(Constants::ProviderDmenu);
            activeSet.name = Constants::ProviderDmenu;
        }
        
        // Defaults for ad-hoc modes
        if (activeSet.providers.contains(Constants::ProviderTop)) {
            activeSet.prompt = "Top > ";
            activeSet.icon = "utilities-system-monitor";
        } else if (activeSet.providers.contains(Constants::ProviderKill)) {
            activeSet.prompt = "Kill > ";
            activeSet.icon = "process-stop"; // or application-exit
        } else if (activeSet.providers.contains(Constants::ProviderSSH)) {
            activeSet.prompt = "SSH > ";
            activeSet.icon = "network-server"; // or computer
        }
    }
    
    // Apply Active Set prompts/overrides
    if (!activeSet.prompt.isEmpty()) {
        engine.rootContext()->setContextProperty("cliPrompt", activeSet.prompt);
    }
    // 2. Resolve default icon (Logo fallback)
    QString defaultIcon = "search";
    QString appIconPath = QCoreApplication::applicationDirPath() + "/assets/logo.png";
    if (QFile::exists(appIconPath)) {
        defaultIcon = appIconPath;
    } else {
        // Check current working directory (dev mode)
        appIconPath = QDir::currentPath() + "/assets/logo.png";
        if (QFile::exists(appIconPath)) {
            defaultIcon = appIconPath;
        } else {
            // Check embedded resource (Nix / Installed mode)
            // Note: qt_add_qml_module adds it under qrc:/qt/qrc/URI/path
            QString resPath = ":/qt/qrc/awelauncher/assets/logo.png";
            if (QFile::exists(resPath)) {
                 defaultIcon = "qrc" + resPath.mid(1); // "qrc:/qt/..."
            } else {
                // Final fallback to system icon if installed
                defaultIcon = "awelaunch";
            }
        }
    }

    engine.rootContext()->setContextProperty("cliIcon", activeSet.icon.isEmpty() ? defaultIcon : activeSet.icon);

    // Apply layout overrides if present
    if (usingSet) {
        QMap<QString, QString> setOverrides;
        if (activeSet.layout.width) setOverrides["window.width"] = QString::number(*activeSet.layout.width);
        if (activeSet.layout.height) setOverrides["window.height"] = QString::number(*activeSet.layout.height);
        if (!activeSet.layout.anchor.isEmpty()) setOverrides["window.anchor"] = activeSet.layout.anchor;
        if (activeSet.layout.margin) setOverrides["window.margin"] = QString::number(*activeSet.layout.margin);
        if (!setOverrides.isEmpty()) {
            Config::instance().setOverrides(setOverrides);
        }
    }

    // 2. Instantiate Providers
    std::vector<LauncherItem> aggregatedItems;
    
    for (const QString& providerName : activeSet.providers) {
        if (providerName == Constants::ProviderRun) {
            auto items = PathProvider::scan();
            aggregatedItems.insert(aggregatedItems.end(), items.begin(), items.end());
        } 
        else if (providerName == Constants::ProviderDrun) {
            auto items = DesktopProvider::scan();
            aggregatedItems.insert(aggregatedItems.end(), items.begin(), items.end());
        }
        else if (providerName == Constants::ProviderTop) {
            int limit = Config::instance().getInt("top.limit", 10);
            QString sortStr = Config::instance().getString("top.sort", "cpu");
            ProcessProvider::SortMode sort = (sortStr == "memory") ? ProcessProvider::MEMORY : ProcessProvider::CPU;
            
            auto items = ProcessProvider::scan(true, limit, sort, false);
            aggregatedItems.insert(aggregatedItems.end(), items.begin(), items.end());
        }
        else if (providerName == Constants::ProviderKill) {
            bool showSystem = Config::instance().getString("kill.show_system", "false") == "true";
            // For kill mode, we list ALL (limit -1), sorted by name? Or Memory? Default to Memory for now strictly to be useful.
            auto items = ProcessProvider::scan(false, -1, ProcessProvider::MEMORY, showSystem);
            aggregatedItems.insert(aggregatedItems.end(), items.begin(), items.end());
        }
        else if (providerName == Constants::ProviderSSH) {
            QString termCmd = Config::instance().getString("ssh.terminal", "");
            bool parseKnown = Config::instance().getString("ssh.parse_known_hosts", "true") == "true";
            auto items = SSHProvider::scan(termCmd, parseKnown);
            aggregatedItems.insert(aggregatedItems.end(), items.begin(), items.end());
        }
        else if (providerName == Constants::ProviderWindow) {
            static WindowProvider* wp = nullptr; 
            if (!wp) {
                wp = new WindowProvider(&app);
                if (wp->initialize()) {
                    controller.setWindowProvider(wp);
                    auto windows = wp->getWindows();
                    std::vector<LauncherItem> wItems(windows.begin(), windows.end());
                    aggregatedItems.insert(aggregatedItems.end(), wItems.begin(), wItems.end());
                } else {
                    delete wp; wp = nullptr;
                }
            }
        }
        else if (providerName == Constants::ProviderDmenu) {
            StdinProvider* stdinProvider = new StdinProvider(&app);
            QObject::connect(stdinProvider, &StdinProvider::itemsChanged, &model, [&model, stdinProvider](){
                auto items = stdinProvider->getItems();
                std::vector<LauncherItem> stdItems(items.begin(), items.end());
                model.setItems(stdItems);
            });
            stdinProvider->start();
            controller.setDmenuMode(true);
        }
    }
    
    // 3. Apply Filters
    if (!activeSet.filter.include.isEmpty() || !activeSet.filter.exclude.isEmpty()) {
         std::vector<LauncherItem> filtered;
         for (const auto& item : aggregatedItems) {
             bool keep = true;
             
             // Check Exclude (High priority)
             if (!activeSet.filter.exclude.isEmpty()) {
                 if (FilterUtils::matches(item.primary, activeSet.filter.exclude) || FilterUtils::matches(item.id, activeSet.filter.exclude)) {
                     keep = false;
                 }
             }
             
             // Check Include (if survived exclude)
             if (keep && !activeSet.filter.include.isEmpty()) {
                 bool included = false;
                 if (FilterUtils::matches(item.primary, activeSet.filter.include) || FilterUtils::matches(item.id, activeSet.filter.include)) {
                     included = true;
                 }
                 if (!included) keep = false;
             }
             
             if (keep) filtered.push_back(item);
         }
         aggregatedItems = filtered;
    }
    
    // Set Items on Model
    if (!parser.isSet(dmenuOption)) {
         model.setItems(aggregatedItems);
    }
    
    // Configure fallback
    Config::instance().validateKeys(); // Ensure all keys valid
    // For now, assuming standard fallback behavior is desired unless disabled
    // If we parsed `general.fallbacks.run_command` from config, we'd use it here.
    // Let's check config string manually for now since we don't have a typed getter for specific deep keys yet
    // Actually, we do have recursion logic.
    // But `Config` logic is: "general.fallbacks.run_command" -> key split...
    // Let's assume defaults for now or parse deeply if time permits.
    // Since `getString` splits by dot, we can try: 
    QString fallbackStr = Config::instance().getString("general.fallbacks.run_command", "true");
    model.setFallbackEnabled(fallbackStr == "true");

    PROFILE_POINT("Items aggregated");

    const QUrl url(QStringLiteral(u"qrc:/awelauncher/src/qml/LauncherRoot.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);
    
    PROFILE_POINT("QML loaded");

    return app.exec();
}
