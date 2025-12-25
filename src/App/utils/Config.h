#pragma once

#include <QString>
#include <yaml-cpp/yaml.h>

class Config
{
public:
    static Config& instance();
    void load(const QString& configPath);
    void ensureDefaults();
    void validateKeys();
    
    QString getString(const QString& key, const QString& defaultValue = "") const;
    int getInt(const QString& key, int defaultValue = 0) const;

private:
    Config() = default;
    YAML::Node m_config;
    QString m_lastPath;
};
