#pragma once

#include <QObject>
#include <QColor>

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

public:
    explicit Theme(QObject *parent = nullptr);

    void load(const QString& themeName);
    
    friend bool loadFromBase16(Theme* theme);

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
};
