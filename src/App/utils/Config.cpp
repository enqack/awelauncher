#include "Config.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QDebug>

Config& Config::instance()
{
    static Config config;
    return config;
}

void Config::ensureDefaults()
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/awelauncher";
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    // Qt QML module resource path
    // URI: awelauncher -> qrc:/awelauncher
    // We added RESOURCES to CMake, which puts them relative to the module root.
    // So examples/config/config.yaml -> qrc:/awelauncher/examples/config/config.yaml
    
    struct ResFile { const char* src; const char* dest; };
    ResFile files[] = {
        { ":/awelauncher/examples/config/config.yaml", "config.yaml" },
        { ":/awelauncher/examples/config/themes/default.yaml", "themes/default.yaml" },
        { ":/awelauncher/examples/config/themes/catppuccin.yaml", "themes/catppuccin.yaml" },
        { ":/awelauncher/examples/config/themes/test-red.yaml", "themes/test-red.yaml" },
        { ":/awelauncher/examples/config/themes/test-blue.yaml", "themes/test-blue.yaml" }
    };
    
    for (const auto& f : files) {
        QString destPath = configDir + "/" + f.dest;
        QFileInfo destInfo(destPath);
        
        if (!destInfo.absoluteDir().exists()) {
            destInfo.absoluteDir().mkpath(".");
        }
        
        if (!QFile::exists(destPath)) {
            // Check if resource exists before copying
            if (QFile::exists(f.src)) {
                qDebug() << "Installing default config:" << destPath;
                QFile::copy(f.src, destPath);
                QFile::setPermissions(destPath, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther);
            } else {
                qWarning() << "Resource missing, cannot install default:" << f.src;
            }
        }
    }
}

void Config::load(const QString& configPath)
{
    if (!QFile::exists(configPath)) {
        qWarning() << "Config file not found:" << configPath << "- using defaults";
        return;
    }
    
    m_lastPath = configPath;
    
    try {
        m_config = YAML::LoadFile(configPath.toStdString());
        qInfo() << "Loaded config from:" << configPath;
        
        if (m_config.IsMap()) {
            qDebug() << "Config loaded successfully. Keys:" << m_config.size();
        } else {
            qWarning() << "Config root is NOT a map!";
        }
    } catch (const YAML::Exception& e) {
        qWarning() << "Failed to parse config file:" << e.what();
    } catch (...) {
        qWarning() << "Unknown error loading config file:" << configPath;
    }
    
    validateKeys();
}
    

void Config::validateKeys() {
    qInfo() << "Validating config keys...";
    if (!m_config.IsDefined() || !m_config.IsMap()) return;
    
    // Whitelist of valid top-level keys
    QStringList validKeys = { "general", "window", "layout" };
    
    for (YAML::const_iterator it = m_config.begin(); it != m_config.end(); ++it) {
        QString key = QString::fromStdString(it->first.as<std::string>());
        if (!validKeys.contains(key)) {
            qWarning() << "[Validation] Unknown top-level key found:" << key;
        } else {
            qInfo() << "[Validation] Found valid key:" << key;
        }
    }
    
    // Validate window section
    if (m_config["window"]) {
        YAML::Node window = m_config["window"];
        if (window.IsMap()) {
            for (YAML::const_iterator it = window.begin(); it != window.end(); ++it) {
                QString key = QString::fromStdString(it->first.as<std::string>());
                if (key != "width" && key != "height") {
                    qWarning() << "[Validation] Unknown key in 'window':" << key;
                }
            }
        } else {
            qWarning() << "[Validation] 'window' must be a map/section";
        }
    }
}

// Use recursion to avoid handle mutation issues resulting from reassignment
// (YAML::Node handles seem to share state in a way that iterative reassignment affects the original)
static YAML::Node resolve(const YAML::Node& node, const QStringList& parts) {
    if (!node.IsDefined() || node.IsNull()) return YAML::Node();
    if (parts.isEmpty()) return node;
    
    QString part = parts.first();
    std::string partStr = part.toStdString();
    
    if (!node.IsMap()) return YAML::Node();
    
    // Check if key exists (read-only)
    YAML::Node next = node[partStr];
    if (!next.IsDefined()) return YAML::Node();
    
    return resolve(next, parts.mid(1));
}

static YAML::Node resolve(const YAML::Node& node, const QString& key) {
    return resolve(node, key.split("."));
}

QString Config::getString(const QString& key, const QString& defaultValue) const
{
    // Check overrides first
    if (m_overrides.contains(key)) {
        return m_overrides.value(key);
    }

    YAML::Node node = resolve(m_config, key);
    if (node.IsDefined() && !node.IsNull()) {
        try {
            return QString::fromStdString(node.as<std::string>());
        } catch (...) { }
    }
    return defaultValue;
}

int Config::getInt(const QString& key, int defaultValue) const
{
    YAML::Node node = resolve(m_config, key);
    if (node.IsDefined() && !node.IsNull()) {
        try {
            return node.as<int>();
        } catch (...) { }
    }
    return defaultValue;
}

QColor Config::getColor(const QString& key, const QColor& defaultValue) const
{
    if (m_overrides.contains(key)) {
        return QColor(m_overrides.value(key));
    }

    YAML::Node node = resolve(m_config, key);
    if (node.IsDefined() && !node.IsNull()) {
        try {
            return QColor(QString::fromStdString(node.as<std::string>()));
        } catch (...) { }
    }
    return defaultValue;
}

void Config::setOverrides(const QMap<QString, QString>& overrides) {
    m_overrides = overrides;
    qInfo() << "Config overrides applied:" << overrides.size();
}
