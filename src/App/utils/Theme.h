#pragma once

#include <QObject>
#include <QColor>

/**
 * @class Theme
 * @brief Manages the visual appearance and layout metrics of the application.
 * 
 * Registered as a QML singleton @c AppTheme. It handles loading themes from
 * YAML files, Base16 environment variables, and applying global configuration 
 * overrides.
 */
class Theme : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QColor bg READ bg NOTIFY themeChanged)
    Q_PROPERTY(QColor fg READ fg NOTIFY themeChanged)
    Q_PROPERTY(QColor accent READ accent NOTIFY themeChanged)
    Q_PROPERTY(QColor selected READ selected NOTIFY themeChanged)
    Q_PROPERTY(QColor muted READ muted NOTIFY themeChanged)
    Q_PROPERTY(QColor hover READ hover NOTIFY themeChanged)
    Q_PROPERTY(QColor border READ border NOTIFY themeChanged)

    // Metrics
    Q_PROPERTY(int padding READ padding NOTIFY themeChanged)
    Q_PROPERTY(int rowHeight READ rowHeight NOTIFY themeChanged)
    Q_PROPERTY(int fontSize READ fontSize NOTIFY themeChanged)
    Q_PROPERTY(int secondaryFontSize READ secondaryFontSize NOTIFY themeChanged)
    Q_PROPERTY(int iconSize READ iconSize NOTIFY themeChanged)
    Q_PROPERTY(int radius READ radius NOTIFY themeChanged)
    Q_PROPERTY(int borderWidth READ borderWidth NOTIFY themeChanged)
    Q_PROPERTY(qreal opacity READ opacity NOTIFY themeChanged)
    Q_PROPERTY(int windowWidth READ windowWidth NOTIFY themeChanged)
    Q_PROPERTY(int windowHeight READ windowHeight NOTIFY themeChanged)
    Q_PROPERTY(QString windowAnchor READ windowAnchor NOTIFY themeChanged)
    Q_PROPERTY(int windowMargin READ windowMargin NOTIFY themeChanged)
    Q_PROPERTY(int windowLayer READ windowLayer NOTIFY themeChanged)

public:
    explicit Theme(QObject *parent = nullptr);


    /**
     * @brief Loads a theme by name.
     * 
     * The loading priority is:
     * 1. Base16 environment (if name is "auto" or empty)
     * 2. YAML file in @c ~/.config/awelauncher/themes/`name`.yaml
     * 3. Built-in hardcoded defaults
     * 
     * Finally, global @c Config overrides are applied on top.
     */
    void load(const QString& themeName);
    
    friend bool loadFromBase16(Theme* theme);
    friend class ThemeScanner;
    

    QColor bg() const { return m_bg; }
    QColor fg() const { return m_fg; }
    QColor accent() const { return m_accent; }
    QColor selected() const { return m_selected; }
    QColor muted() const { return m_muted; }
    QColor hover() const { return m_hover; }
    QColor border() const { return m_border; }

    int padding() const { return m_padding; }
    int rowHeight() const { return m_rowHeight; }
    int fontSize() const { return m_fontSize; }
    int secondaryFontSize() const { return m_secondaryFontSize; }
    int iconSize() const { return m_iconSize; }
    int radius() const { return m_radius; }
    int borderWidth() const { return m_borderWidth; }
    qreal opacity() const { return m_opacity; }
    int windowWidth() const { return m_windowWidth; }
    int windowHeight() const { return m_windowHeight; }
    QString windowAnchor() const { return m_windowAnchor; }
    int windowMargin() const { return m_windowMargin; }
    int windowLayer() const { return m_windowLayer; }

signals:
    void themeChanged();

private:
    QColor m_bg;
    QColor m_fg;
    QColor m_accent;
    QColor m_selected;
    QColor m_muted;
    QColor m_hover;
    QColor m_border;
    
    int m_padding = 20;
    int m_rowHeight = 48;
    int m_fontSize = 16;
    int m_secondaryFontSize = 13;
    int m_iconSize = 32;
    int m_radius = 16;
    int m_borderWidth = 1;
    qreal m_opacity = 0.95;
    int m_windowWidth = 600;
    int m_windowHeight = 400;
    QString m_windowAnchor = "center";
    int m_windowMargin = 0;
    int m_windowLayer = 1; // Default to LayerTop
};
