#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtQml>
#include <QCommandLineParser>
#include <QElapsedTimer>
#include "App/controllers/LauncherController.h"
#include "App/models/LauncherModel.h"
#include "App/utils/Theme.h"
#include "App/providers/IconProvider.h"
#include "App/providers/DesktopFileLoader.h"
#include "App/providers/WindowProvider.h"

#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include "App/utils/Config.h"

// Profiling helper
#define PROFILE_POINT(name) \
    if (debugMode) qDebug() << "[PROFILE]" << name << ":" << timer.elapsed() << "ms";

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
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
    
    QCommandLineOption debugOption(QStringList() << "d" << "debug",
        "Enable debug output");
    parser.addOption(debugOption);

    QCommandLineOption clearCacheOption(QStringList() << "c" << "clear-cache",
        "Clear icon cache on startup");
    parser.addOption(clearCacheOption);
    
    parser.process(app);
    
    // Start profiling timer
    bool debugMode = parser.isSet(debugOption);
    QElapsedTimer timer;
    timer.start();

    if (parser.isSet(clearCacheOption)) {
        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/awelauncher/icons";
        QDir dir(cacheDir);
        if (dir.exists()) {
            dir.removeRecursively();
            qDebug() << "Cleared icon cache:" << cacheDir;
        }
    }
    
    PROFILE_POINT("App init");

    // Load Config
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/awelauncher/config.yaml";
    Config::instance().ensureDefaults();
    Config::instance().load(configPath);
    
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
    } else {
        // Load desktop applications (drun mode)
        model.setItems(DesktopFileLoader::scan());
    }
    
    PROFILE_POINT("Items loaded");

    const QUrl url(u"qrc:/awelauncher/src/qml/LauncherRoot.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);
    
    PROFILE_POINT("QML loaded");

    return app.exec();
}
