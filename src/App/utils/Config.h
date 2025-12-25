#pragma once

#include <QString>
#include <QMap>
#include <QColor>
#include <yaml-cpp/yaml.h>

/**
 * @class Config
 * @brief Singleton class for managing application-wide configuration.
 * 
 * It handles loading YAML configuration, ensuring default settings are 
 * installed, and providing type-safe access to configuration values with
 * support for command-line overrides.
 */
class Config
{
public:
    /** @brief Main access point for the singleton instance. */
    static Config& instance();
    /** @brief Loads configuration from the specified YAML file. */
    void load(const QString& configPath);
    /** @brief Installs default config and theme files if they don't exist. */
    void ensureDefaults();
    /** @brief Validates the structure and keys of the loaded YAML. */
    void validateKeys();
    
    /** @brief Retrieves a string value by key (e.g. "general.theme"). */
    QString getString(const QString& key, const QString& defaultValue = "") const;
    /** @brief Retrieves an integer value by key (e.g. "window.width"). */
    int getInt(const QString& key, int defaultValue = 0) const;
    
    /** @brief Retrieves a color value by key (e.g. "colors.bg"). */
    QColor getColor(const QString& key, const QColor& defaultValue) const;

    /** @brief Sets manual overrides (usually from CLI arguments). */
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
