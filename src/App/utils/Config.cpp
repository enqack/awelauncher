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
    
    // Load Global Pins & Aliases
    if (m_config["general"].IsDefined()) {
        YAML::Node general = m_config["general"];
        
        // Pins
        if (general["pins"].IsDefined() && general["pins"].IsSequence()) {
            m_globalPins.clear();
            for (const auto& item : general["pins"]) {
                m_globalPins.append(QString::fromStdString(item.as<std::string>()));
            }
        }
        
        // Aliases
        if (general["aliases"].IsDefined() && general["aliases"].IsSequence()) {
            m_globalAliases.clear();
            for (const auto& item : general["aliases"]) {
                if (item["name"].IsDefined() && item["target"].IsDefined()) {
                    m_globalAliases.insert(
                        QString::fromStdString(item["name"].as<std::string>()),
                        QString::fromStdString(item["target"].as<std::string>())
                    );
                }
            }
        }
    }

    // Load sets
    if (m_config["sets"].IsDefined() && m_config["sets"].IsMap()) {
        YAML::Node setsNode = m_config["sets"];
        for (YAML::const_iterator it = setsNode.begin(); it != setsNode.end(); ++it) {
            QString setName = QString::fromStdString(it->first.as<std::string>());
            YAML::Node setNode = it->second;
            
            ProviderSet set;
            set.name = setName;
            
            // prompt
            if (setNode["prompt"].IsDefined()) {
                set.prompt = QString::fromStdString(setNode["prompt"].as<std::string>());
            }
            
            // icon
            if (setNode["icon"].IsDefined()) {
                set.icon = QString::fromStdString(setNode["icon"].as<std::string>());
            }
            
            // providers
            if (setNode["providers"].IsDefined() && setNode["providers"].IsSequence()) {
                for (const auto& p : setNode["providers"]) {
                    set.providers.append(QString::fromStdString(p.as<std::string>()));
                }
            }
            
            // pins
            if (setNode["pins"].IsDefined() && setNode["pins"].IsSequence()) {
                for (const auto& item : setNode["pins"]) {
                    set.pins.append(QString::fromStdString(item.as<std::string>()));
                }
            }
            
            // aliases
            if (setNode["aliases"].IsDefined() && setNode["aliases"].IsSequence()) {
                for (const auto& item : setNode["aliases"]) {
                     if (item["name"].IsDefined() && item["target"].IsDefined()) {
                        set.aliases.insert(
                            QString::fromStdString(item["name"].as<std::string>()),
                            QString::fromStdString(item["target"].as<std::string>())
                        );
                    }
                }
            }
            
            // layout overrides
            if (setNode["layout"].IsDefined()) {
                YAML::Node l = setNode["layout"];
                if (l["width"].IsDefined()) set.layout.width = l["width"].as<int>();
                if (l["height"].IsDefined()) set.layout.height = l["height"].as<int>();
                if (l["anchor"].IsDefined()) set.layout.anchor = QString::fromStdString(l["anchor"].as<std::string>());
                if (l["margin"].IsDefined()) set.layout.margin = l["margin"].as<int>();
            }
            
            // filter
            if (setNode["filter"].IsDefined()) {
                YAML::Node f = setNode["filter"];
                // ... extract logic ...
                auto extractMapValues = [&](const YAML::Node& node) -> QStringList {
                    QStringList res;
                    if (node.IsMap()) {
                        for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
                            if (it->second.IsSequence()) {
                                for (const auto& v : it->second) res.append(QString::fromStdString(v.as<std::string>()));
                            } else {
                                res.append(QString::fromStdString(it->second.as<std::string>()));
                            }
                        }
                    }
                    return res;
                };

                if (f["include"].IsDefined()) {
                    set.filter.include = extractMapValues(f["include"]);
                }
                if (f["exclude"].IsDefined()) {
                    set.filter.exclude = extractMapValues(f["exclude"]);
                }
            }
            
            m_sets.insert(setName, set);
        }
    }
    
    validateKeys();
}
    

void Config::validateKeys() {
    qInfo() << "Validating config keys...";
    if (!m_config.IsDefined() || !m_config.IsMap()) return;
    
    
    // Whitelist of valid top-level keys
    QStringList validKeys = { "general", "window", "layout", "sets", "top", "kill", "ssh" };
    
    // ... validation loop ...
    
    // Check general.empty_state and fallbacks
    // (Actual usage is via getString/getBool in code, so validation could be looser or stricter)

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
                if (key != "width" && key != "height" && key != "anchor" && key != "margin" && key != "layer") {
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
    if (m_overrides.contains(key)) {
        bool ok;
        int val = m_overrides.value(key).toInt(&ok);
        if (ok) return val;
    }

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

std::optional<Config::ProviderSet> Config::getSet(const QString& name) const
{
    if (m_sets.contains(name)) {
        return m_sets.value(name);
    }
    return std::nullopt;
}
