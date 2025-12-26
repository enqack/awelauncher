#include <QtQuickTest/quicktest.h>
#include <QQmlEngine>
#include <QQmlContext>
#include "App/utils/Theme.h"

#include <QObject>

class MockController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString promptOverride READ promptOverride CONSTANT)
public:
    explicit MockController(QObject* parent = nullptr) : QObject(parent) {}
    QString promptOverride() { return ""; }
    Q_INVOKABLE void filter(const QString&) {}
    Q_INVOKABLE void activate(int) {}
};

class Setup : public QObject
{
    Q_OBJECT

public:
    Setup() {}

public slots:
    void qmlEngineAvailable(QQmlEngine *engine)
    {
        static Theme theme;
        // Make sure to load default theme or base16 so properties are valid
        theme.load("default");
        
        qmlRegisterSingletonInstance("awelauncher", 1, 0, "AppTheme", &theme);
        
        engine->rootContext()->setContextProperty("cliShowMode", "drun");
        engine->rootContext()->setContextProperty("cliPrompt", "Test Prompt");
        engine->rootContext()->setContextProperty("cliIcon", "search");
        engine->rootContext()->setContextProperty("debugMode", true);
        
        static MockController mockController;
        engine->rootContext()->setContextProperty("Controller", &mockController);
    }
};

QUICK_TEST_MAIN_WITH_SETUP(test_qml, Setup)

#include "tst_main.moc"
