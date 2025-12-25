#include "Theme.h"
#include "Config.h"
#include <QStandardPaths>
#include <QFile>
#include <QDebug>
#include <yaml-cpp/yaml.h>

Theme::Theme(QObject *parent)
    : QObject(parent)
{
    // Hardcoded defaults for MVP (Fallback)
    m_bg = QColor("#1e1e2e");
    m_fg = QColor("#cdd6f4");
    m_accent = QColor("#89b4fa");
    m_selected = QColor("#313244");
    m_muted = QColor("#6c7086");
    m_hover = QColor("#45475a");
    m_border = QColor("#6c7086");
}

static QString getStr(const YAML::Node& node, const char* key, QString fallback) {
    if (node[key]) return QString::fromStdString(node[key].as<std::string>());
    return fallback;
}

static int getIntVal(const YAML::Node& node, const char* key, int fallback) {
    if (node[key]) return node[key].as<int>();
    return fallback;
}

bool loadFromBase16(Theme* theme) {
    // Check for base16-shell environment variables
    QByteArray base00 = qgetenv("BASE16_COLOR_00_HEX");
    QByteArray base01 = qgetenv("BASE16_COLOR_01_HEX");
    QByteArray base03 = qgetenv("BASE16_COLOR_03_HEX");
    QByteArray base04 = qgetenv("BASE16_COLOR_04_HEX");
    QByteArray base05 = qgetenv("BASE16_COLOR_05_HEX");
    QByteArray base0D = qgetenv("BASE16_COLOR_0D_HEX");
    
    // If base16 vars aren't set, return false
    if (base00.isEmpty() || base05.isEmpty()) {
        return false;
    }
    
    qDebug() << "Loading theme from base16 environment variables";
    
    // Map base16 colors to our theme
    // base00 = background, base05 = foreground, base0D = accent
    // base01 = lighter bg (selected), base03/04 = muted
    theme->m_bg = QColor("#" + QString(base00));
    theme->m_fg = QColor("#" + QString(base05));
    theme->m_accent = QColor("#" + QString(base0D.isEmpty() ? base05 : base0D));
    theme->m_selected = QColor("#" + QString(base01.isEmpty() ? base00 : base01));
    theme->m_muted = QColor("#" + QString(base04.isEmpty() ? base03 : base04));
    
    return true;
}


void Theme::load(const QString& themeName)
{
    // Priority 1: Try base16 environment variables (system theme)
    if (themeName == "auto" || themeName == "system" || themeName.isEmpty()) {
        if (loadFromBase16(this)) {
            emit themeChanged();
            return;
        }
        qDebug() << "Base16 env vars not found, falling back to default theme";
        // Fall back to default theme instead of looking for "auto.yaml"
        load("default");
        return;
    }
    
    // Priority 2: Load from YAML file
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QString themePath = configDir + "/awelauncher/themes/" + themeName + ".yaml";

    if (!QFile::exists(themePath)) {
        qWarning() << "Theme file not found:" << themePath << "- keeping defaults";
        return;
    }

    try {
        YAML::Node theme = YAML::LoadFile(themePath.toStdString());
        qDebug() << "Loading theme from:" << themePath;

        if (auto colors = theme["colors"]) {
            m_bg = QColor(getStr(colors, "bg", m_bg.name()));
            m_fg = QColor(getStr(colors, "fg", m_fg.name()));
            m_accent = QColor(getStr(colors, "accent", m_accent.name()));
            m_selected = QColor(getStr(colors, "selected", m_selected.name()));
            m_muted = QColor(getStr(colors, "muted", m_muted.name()));
        }

        if (auto layout = theme["layout"]) {
            m_padding = getIntVal(layout, "padding", m_padding);
            m_rowHeight = getIntVal(layout, "rowHeight", m_rowHeight);
            m_fontSize = getIntVal(layout, "fontSize", m_fontSize);
            m_iconSize = getIntVal(layout, "iconSize", m_iconSize);
            m_radius = getIntVal(layout, "radius", m_radius);
            
            // Parse opacity (0.0 - 1.0)
            if (layout["opacity"]) {
                m_opacity = layout["opacity"].as<double>();
                // Clamp to valid range
                if (m_opacity < 0.0) m_opacity = 0.0;
                if (m_opacity > 1.0) m_opacity = 1.0;
            }
        }
        
        if (auto window = theme["window"]) {
            m_windowWidth = getIntVal(window, "width", m_windowWidth);
            m_windowHeight = getIntVal(window, "height", m_windowHeight);
            m_windowMargin = getIntVal(window, "margin", m_windowMargin);
            
            QString layer = getStr(window, "layer", "top");
            if (layer == "overlay") m_windowLayer = 2; // LayerOverlay
            else m_windowLayer = 1; // LayerTop
        }

        // emit themeChanged(); // Deferred to end of function
        
    } catch (const YAML::Exception& e) {
        qWarning() << "Failed to parse theme file:" << e.what();
    } catch (...) {
        qWarning() << "Unknown error loading theme:" << themePath;
    }


    // Priority 0: Global Config Overrides
    // Always apply these on top of whatever theme was loaded
    Config& c = Config::instance();
    m_padding = c.getInt("layout.padding", m_padding);
    m_rowHeight = c.getInt("layout.rowHeight", m_rowHeight);
    m_fontSize = c.getInt("layout.fontSize", m_fontSize);
    m_iconSize = c.getInt("layout.iconSize", m_iconSize);
    m_radius = c.getInt("layout.radius", m_radius);
    
    // Opacity override is a bit trickier since we don't have getDouble yet, 
    // but the Config class uses YAML::Node so we can add getDouble or just hack it for now.
    // Let's assume valid range if provided.
    // For now, let's just stick to integer overrides or add getDouble to Config if needed.
    // Wait, Config doesn't have getDouble. Let's add it or rely on int/string parsing.
    // Simpler: Just override metrics for now.
    
    // Window size overrides
    qInfo() << "Checking config for window overrides...";


    m_windowWidth = c.getInt("window.width", m_windowWidth);
    m_windowHeight = c.getInt("window.height", m_windowHeight);
    m_windowAnchor = c.getString("window.anchor", m_windowAnchor);
    m_windowMargin = c.getInt("window.margin", m_windowMargin);
    
    QString layerOverride = c.getString("window.layer", "");
    if (!layerOverride.isEmpty()) {
        if (layerOverride == "overlay") m_windowLayer = 2;
        else m_windowLayer = 1;
    }
    
    qInfo() << "Final Window Size:" << m_windowWidth << "x" << m_windowHeight << "Anchor:" << m_windowAnchor << "Margin:" << m_windowMargin << "Layer:" << (m_windowLayer == 2 ? "Overlay" : "Top");

    emit themeChanged();
}
