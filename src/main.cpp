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
#include <QWindow>
#include <QCommandLineParser>
#include <QElapsedTimer>
#include <QSurfaceFormat>
#include "App/controllers/LauncherController.h"
#include "App/controllers/DaemonController.h"
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
#include <QDirIterator>
#include <QDebug>
#include <QIcon>
#include "App/utils/Config.h"
#include "App/utils/FilterUtils.h"
#include "App/utils/Constants.h"
#include "App/utils/OutputUtils.h"

#include "App/utils/Profiler.h"

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

    QCommandLineOption daemonOption("daemon", "Start in background daemon mode");
    parser.addOption(daemonOption);

    QCommandLineOption queryOption(QStringList() << "q" << "query", "Query the daemon for results and exit (JSON)", "text");
    parser.addOption(queryOption);

    // RFC-003: Monitor Destinations
    QCommandLineOption monitorOption(QStringList() << "monitor", "Strategy to select monitor (follow-mouse, follow-focus, or name)", "strategy");
    parser.addOption(monitorOption);

    QCommandLineOption outputOption(QStringList() << "output", "Target specific output by name (e.g. DP-1)", "name");
    parser.addOption(outputOption);

    parser.process(app);

    // Initial Debug / Profiling setup
    bool debugMode = parser.isSet(debugOption);
    Config::instance().setDebug(debugMode);
    qInstallMessageHandler(personalMessageHandler);

    if (debugMode) {
        qDebug() << "[Diagnostics] Platform Name:" << app.platformName();
        qDebug() << "[Diagnostics] QT_WAYLAND_SHELL_INTEGRATION:" << qgetenv("QT_WAYLAND_SHELL_INTEGRATION");
    }

    // --- Client / Daemon Logic ---
    QString socketPath = DaemonController::socketPath();
    bool daemonRunning = QFile::exists(socketPath);
    bool startDaemon = parser.isSet(daemonOption);

    if (daemonRunning && !startDaemon) {
        // Mode: Client (External trigger or query)
        QLocalSocket socket;
        socket.connectToServer(socketPath);
        if (socket.waitForConnected(500)) {
            QJsonObject cmd;
            cmd["version"] = 1;
            
            if (parser.isSet(queryOption)) {
                cmd["action"] = "query";
                QJsonObject payload;
                payload["text"] = parser.value(queryOption);
                cmd["payload"] = payload;
            } else {
                cmd["action"] = "show";
                QJsonObject payload;
                if (parser.isSet(setOption)) payload["set"] = parser.value(setOption);
                if (parser.isSet(showOption)) payload["mode"] = parser.value(showOption);
                if (parser.isSet(promptOption)) payload["prompt"] = parser.value(promptOption);
                cmd["payload"] = payload;
            }
            
            socket.write(QJsonDocument(cmd).toJson(QJsonDocument::Compact));
            socket.flush();
            
            if (parser.isSet(queryOption)) {
                if (socket.waitForReadyRead(1000)) {
                    printf("%s\n", socket.readAll().constData());
                    APP_PROFILE_POINT(timer, "Query response received");
                } else {
                    fprintf(stderr, "Error: Timeout waiting for daemon response\n");
                }
            } else {
                APP_PROFILE_POINT(timer, "Daemon show command sent");
            }
            return 0;
        } else {
            // Socket exists but unresponsive - cleanup and proceed to standalone/daemon
            QLocalServer::removeServer(socketPath);
        }
    }

    if (startDaemon) {
        // Mode: Daemon Start
        // We'll initialize the full engine but hide the window by default
        qDebug() << "Starting in daemon mode...";
    }

    if (parser.isSet(clearCacheOption)) {
        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/awelauncher/icons";
        QDir dir(cacheDir);
        if (dir.exists()) {
            dir.removeRecursively();
            if (debugMode) qDebug() << "Cleared icon cache:" << cacheDir;
        }
    }
    
    APP_PROFILE_POINT(timer, "App init");

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
    
    APP_PROFILE_POINT(timer, "Config loaded");

    QQmlApplicationEngine engine;

    // Register singletons/providers
    
    // Icon provider
    engine.addImageProvider("icon", new IconProvider());

    // Core Controller & Model (Heap allocated to ensure stable pointers)
    auto *controller = new LauncherController(&app);
    auto *model = new LauncherModel(&app);
    
    engine.rootContext()->setContextProperty("launcher", controller);
    engine.rootContext()->setContextProperty("LauncherModel", model);
    engine.rootContext()->setContextProperty("debugMode", debugMode);

    // Daemon/IPC setup
    auto *daemon = new DaemonController(controller, &app);
    if (startDaemon) {
        controller->setDaemonMode(true);
        controller->setVisible(false); // Hide window in daemon mode
    }
    daemon->start();

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
    
    APP_PROFILE_POINT(timer, "Theme loaded");

    qmlRegisterSingletonInstance("awelauncher", 1, 0, "AppTheme", &theme);

    // Connect logic
    controller->setModel(model);

    // Initialize WindowProvider if possible
    // Initialize WindowProvider if possible
    WindowProvider* wp = new WindowProvider(&app);
    if (wp->initialize()) {
        controller->setWindowProvider(wp);
    } else {
        delete wp;
    }

    // --- Provider Aggregation Logic ---
    
    // Determine which Set/Mode to use from CLI
    QString setName = parser.value(setOption);
    QString showMode = parser.value(showOption);
    
    // Special case: if dmenu flag is on, force dmenu provider (Standalone only)
    if (parser.isSet(dmenuOption)) {
        StdinProvider* stdinProvider = new StdinProvider(&app);
        QObject::connect(stdinProvider, &StdinProvider::itemsChanged, model, [model, stdinProvider](){
            auto items = stdinProvider->getItems();
            std::vector<LauncherItem> stdItems(items.begin(), items.end());
            model->setItems(stdItems);
        });
        stdinProvider->start();
        controller->setDmenuMode(true);
    } else {
        // Load initial set
        controller->loadSet(setName, showMode);
    }
    
    // Configure fallback
    Config::instance().validateKeys();
    QString fallbackStr = Config::instance().getString("general.fallbacks.run_command", "true");
    model->setFallbackEnabled(fallbackStr == "true");

    // Set Window Icon
    QString iconStr = controller->icon();
    if (iconStr.startsWith("qrc:/") || iconStr.startsWith(":/") || iconStr.startsWith("/")) {
        app.setWindowIcon(QIcon(iconStr));
    } else {
        app.setWindowIcon(QIcon::fromTheme(iconStr));
    }

    APP_PROFILE_POINT(timer, "Items aggregated");

    const QUrl url(QStringLiteral(u"qrc:/awelauncher/src/qml/LauncherRoot.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    // Lazy Load UI if in daemon mode
    if (startDaemon) {
        controller->setUiInitializer([&engine, url]() {
             engine.load(url);
             // We can capture timer from main scope if needed, but it might be tricky with QElapsedTimer copy.
             // Just log directly or use APP_PROFILE_POINT if we re-instantiate timer?
             // Actually timer is in main stack, capturing by ref is safe as main blocks.
             // But APP_PROFILE_POINT uses 'timer' name.
             // Let's just log.
             qDebug() << "[PROFILE] QML loaded (Lazy) : " << QDateTime::currentMSecsSinceEpoch(); 
        });
        APP_PROFILE_POINT(timer, "Daemon Ready (UI Deferred)");
    } else {
        // RFC-003: Resolve Monitor
        QString monitorStrategy = parser.value(monitorOption);
        if (monitorStrategy.isEmpty() && parser.isSet(outputOption)) {
            monitorStrategy = parser.value(outputOption);
        }
        
        // If strategy is provided and NOT follow-focus, or if we have an output name, it's explicit
        bool isExplicit = !monitorStrategy.isEmpty() && monitorStrategy != "follow-focus";
        
        // Follow-mouse is considered explicit because we calculate it manually via OutputUtils
        // Wait, if QCursor::pos() is bad, follow-mouse might fall back to primary. 
        // But if user explicitly ASKED for follow-mouse, we should try to honor it manually if possible,
        // or let compositor do it if we implemented it via LayerShell.
        // Actually, LayerShell doesn't have "FollowMouse". So we MUST compute it manually.
        
        if (monitorStrategy.isEmpty()) {
             // Load from config, default to follow-focus
             QString configMonitor = Config::instance().getString("general.monitor", "follow-focus");
             if (configMonitor == "follow-focus") isExplicit = false;
             else {
                 monitorStrategy = configMonitor; 
                 isExplicit = true;
             }
        }
        
        controller->setExplicitScreen(isExplicit);

        QScreen* targetScreen = nullptr;
        if (isExplicit) {
            targetScreen = OutputUtils::resolveScreen(monitorStrategy);
            // If resolution failed (e.g. follow-mouse couldn't find cursor), revert to compositor
            if (!targetScreen) {
                if (debugMode) qDebug() << "Monitor resolution failed for strategy:" << monitorStrategy << "- Reverting to Compositor placement";
                isExplicit = false;
                controller->setExplicitScreen(false);
            }
        }

        engine.load(url);
        
        if (!engine.rootObjects().isEmpty()) {
            QWindow *window = qobject_cast<QWindow *>(engine.rootObjects().first());
            if (window) {
                controller->setMainWindow(window); // Always pass window for focus handling
                
                if (isExplicit && targetScreen) {
                    window->setScreen(targetScreen);
                    if (debugMode) qDebug() << "Set window screen to:" << targetScreen->name();
                }
            }
        }
        
        // RFC-003: Now verify/force visibility for standalone mode
        // Since we changed default to false to allow proper screen placement
        if (!startDaemon) {
             controller->setVisible(true);
        }

        APP_PROFILE_POINT(timer, "QML loaded");
    }

    return app.exec();
}
