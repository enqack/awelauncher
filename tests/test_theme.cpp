
#include <QtTest>
#include "App/utils/Theme.h"
#include "App/utils/Config.h"

// Define a simple mock for ThemeScanner or just use environment vars for base16 since Theme supports that.
// We'll use the environment variable approach as it's built-in to Theme::loadFromBase16

class TestTheme : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        // Ensure clean config state
        // In a real app we might need to be careful about singleton state, but for this test process it should be fine.
    }

    void testGlobalOverridesOnAutoTheme() {
        // 1. Setup Environment for Base16 (Simulate Auto Detection)
        qputenv("BASE16_COLOR_00_HEX", "000000"); // Black bg
        qputenv("BASE16_COLOR_05_HEX", "ffffff"); // White fg
        
        // 2. Setup Global Config File (Real scenario)
        QTemporaryDir tempDir;
        Q_ASSERT(tempDir.isValid());
        
        QString configPath = tempDir.filePath("config.yaml");
        QFile file(configPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "Failed to open config file:" << file.errorString();
            QFAIL("Failed to create config file");
        }
        QTextStream out(&file);
        out << "layout:\n";
        out << "  padding: 999\n";
        out << "window:\n";
        out << "  width: 1234\n";
        file.close();
        
        // Load the config file
        Config::instance().load(configPath);
        
        // 3. Load Theme "auto"
        Theme theme;
        theme.load("auto");
        
        // 4. Verify Colors (should be from env vars)
        QCOMPARE(theme.bg().name(), "#000000");
        QCOMPARE(theme.fg().name(), "#ffffff");
        
        // 5. Verify Overrides
        QCOMPARE(theme.padding(), 999);
        QCOMPARE(theme.windowWidth(), 1234);
    }
    
};

QTEST_MAIN(TestTheme)
#include "test_theme.moc"
