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
        { ":/awelauncher/examples/config/themes/catppuccin.yaml", "themes/catppuccin.yaml" }
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

// Use const YAML::Node& to prevent autovivification
static YAML::Node resolve(const YAML::Node& node, const QString& key) {
    if (!node.IsDefined()) {
        qWarning() << "Resolve called with undefined node for key:" << key;
        return YAML::Node();
    }
    if (node.IsNull()) {
        qWarning() << "Resolve called with null node for key:" << key;
        return YAML::Node();
    }
    
    QStringList parts = key.split(".");
    YAML::Node current = node; // Copy of handle, but checks will be const if we use const methods? 
                               // Actually, in yaml-cpp, copying a const Node yields a Node that might be non-const if not careful?
                               // No, we should keep 'current' as valid node.
                               
    // Iterate carefully
    for (const QString& part : parts) {
        std::string partStr = part.toStdString();
        
        if (!current.IsMap()) return YAML::Node();
        
        YAML::Node next = current[partStr];
        if (!next.IsDefined()) {
             return YAML::Node();
        }
        current = next;
    }
    return current;
}

QString Config::getString(const QString& key, const QString& defaultValue) const
{
    // Sanity check: If we have a path but config root looks wrong (e.g. missing "general" or "window"), reload.
    // This protects against weird yaml-cpp handle corruption.
    if (!m_lastPath.isEmpty() && m_config.IsMap()) {
        if (!m_config["general"] && !m_config["window"]) {
             qDebug() << "Config self-healing: Reloading from" << m_lastPath;
             const_cast<Config*>(this)->load(m_lastPath);
        }
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
    // Sanity check: If we have a path but config root looks wrong (e.g. missing "general" or "window"), reload.
    if (!m_lastPath.isEmpty() && m_config.IsMap()) {
        if (!m_config["general"] && !m_config["window"]) {
             qDebug() << "Config self-healing: Reloading from" << m_lastPath;
             const_cast<Config*>(this)->load(m_lastPath);
        }
    }

    YAML::Node node = resolve(m_config, key);
    if (node.IsDefined() && !node.IsNull()) {
        try {
            return node.as<int>();
        } catch (...) { }
    }
    return defaultValue;
}

