#pragma once

#include <QString>
#include <QMap>
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
    
    void setOverrides(const QMap<QString, QString>& overrides);
    
    void setDebug(bool debug) { m_debug = debug; }
    bool isDebug() const { return m_debug; }

private:
    Config() = default;

    YAML::Node m_config;
    QString m_lastPath;
    QMap<QString, QString> m_overrides;
    bool m_debug = false;
};
