#include "Config.h"
#include <QFile>
#include <QDebug>

Config& Config::instance()
{
    static Config config;
    return config;
}

void Config::load(const QString& configPath)
{
    if (!QFile::exists(configPath)) {
        qWarning() << "Config file not found:" << configPath << "- using defaults";
        return;
    }
    try {
        m_config = YAML::LoadFile(configPath.toStdString());
        qDebug() << "Loaded config from:" << configPath;
    } catch (const YAML::Exception& e) {
        qWarning() << "Failed to parse config file:" << e.what();
    } catch (...) {
        qWarning() << "Unknown error loading config file:" << configPath;
    }
}

static YAML::Node resolve(YAML::Node node, const QString& key) {
    if (!node.IsDefined() || node.IsNull()) return YAML::Node();
    
    QStringList parts = key.split(".");
    YAML::Node current = node;
    
    for (const QString& part : parts) {
        if (!current.IsMap()) return YAML::Node();
        current = current[part.toStdString()];
        if (!current.IsDefined()) return YAML::Node();
    }
    return current;
}

QString Config::getString(const QString& key, const QString& defaultValue) const
{
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
