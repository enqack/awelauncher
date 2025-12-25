#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtQml>
#include <QCommandLineParser>
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


#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include "App/utils/Config.h"

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
    parser.setApplicationDescription("A Wayland-first launcher in QtQuick + C++");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption showOption(QStringList() << "s" << "show",
        "Show specific provider mode: drun, run, or window", "mode", "drun");
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
    engine.addImageProvider("icons", new IconProvider());

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

    // Load items based on show mode
    if (showMode == "run") {
        // Load PATH executables
        std::vector<LauncherItem> pathItems;
        QStringList pathDirs = QString(qgetenv("PATH")).split(":", Qt::SkipEmptyParts);
        
        for (const QString& dir : pathDirs) {
            QDir d(dir);
            if (!d.exists()) continue;
            
            QFileInfoList files = d.entryInfoList(QDir::Files | QDir::Executable, QDir::Name);
            for (const QFileInfo& file : files) {
                LauncherItem item;
                item.id = "path:" + file.fileName();
                item.primary = file.fileName();
                item.secondary = file.absoluteFilePath();
                item.iconKey = "application-x-executable";
                item.exec = file.fileName();
                item.terminal = false;
                item.selected = false;
                pathItems.push_back(item);
            }
        }
        model.setItems(pathItems);
    } else if (showMode == "window") {
        // Load windows from Wayland
        WindowProvider* windowProvider = new WindowProvider(&app);
        if (windowProvider->initialize()) {
            auto windows = windowProvider->getWindows();
            std::vector<LauncherItem> windowItems(windows.begin(), windows.end());
            model.setItems(windowItems);
            controller.setWindowProvider(windowProvider);
            qDebug() << "Loaded" << windows.size() << "windows";
        } else {
            qWarning() << "Failed to initialize window provider";
            delete windowProvider;
        }
    } else if (parser.isSet(dmenuOption)) {
        // dmenu mode: read stdin
        StdinProvider* stdinProvider = new StdinProvider(&app);
        
        QObject::connect(stdinProvider, &StdinProvider::itemsChanged, &model, [&model, stdinProvider](){
            auto items = stdinProvider->getItems();
            std::vector<LauncherItem> stdItems(items.begin(), items.end());
            model.setItems(stdItems);
        });
        
        stdinProvider->start();
        
        controller.setDmenuMode(true);
    } else {
        // Load desktop applications (drun mode)
        auto items = DesktopFileLoader::scan();
        qDebug() << "[Provider] Drun mode found" << items.size() << "items";
        model.setItems(items);
    }

    
    PROFILE_POINT("Items loaded");

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
