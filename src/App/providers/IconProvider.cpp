#include "IconProvider.h"
#include <QRunnable>
#include <QPainter>
#include <QThread>
#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QDateTime>
#include <QIcon>
#include <QPixmap>
#include <QDirIterator>
#include <QPainterPath>

static void applyRounding(QImage& img, float radiusRatio = 0.25) {
    if (img.isNull()) return;
    
    QImage rounded(img.size(), QImage::Format_ARGB32);
    rounded.fill(Qt::transparent);
    
    QPainter p(&rounded);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    
    QBrush brush(img);
    p.setBrush(brush);
    p.setPen(Qt::NoPen);
    
    int rw = img.width() * radiusRatio;
    int rh = img.height() * radiusRatio;
    p.drawRoundedRect(img.rect(), rw, rh);
    p.end();
    
    img = rounded;
}

class IconRunner : public QRunnable
{
public:
    IconRunner(IconResponse *resp) : m_resp(resp) {}
    void run() override {
        m_resp->run();
    }
private:
    IconResponse *m_resp;
};

IconResponse::IconResponse(const QString &id) : m_id(id)
{
}

void IconResponse::run()
{
    int size = 128; // High res
    
    // Check cache first
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/awelauncher/icons";
    QDir().mkpath(cacheDir);
    
    // Create cache key from id + size + timestamp (if file)
    // v2: includes rounding fix
    QString cacheKey = QString("v2_%1_%2").arg(m_id).arg(size);
    if (m_id.startsWith("/") && QFile::exists(m_id)) {
        cacheKey += QString::number(QFileInfo(m_id).lastModified().toMSecsSinceEpoch());
    }
    QByteArray hash = QCryptographicHash::hash(cacheKey.toUtf8(), QCryptographicHash::Md5).toHex();
    QString cachePath = cacheDir + "/" + QString(hash) + ".png";
    
    // Try loading from cache
    if (QFile::exists(cachePath)) {
        m_image = QImage(cachePath);
        if (!m_image.isNull()) {
            emit finished();
            return;
        }
    }
    
    // Generate icon (cache miss)
    
    // 1. Try absolute path or resource
    if (m_id.startsWith("/")) {
        if (m_image.load(m_id)) {
             applyRounding(m_image);
        }
    } else if (m_id.startsWith("qrc:/")) {
        QString resPath = m_id.mid(3); // "qrc:/..." -> ":/..."
        if (m_image.load(resPath)) {
            applyRounding(m_image);
        } else {
             // Try alternative without /qt/qrc/
             QString altPath = resPath;
             altPath.replace("/qt/qrc/", "/");
             if (m_image.load(altPath)) {
                 applyRounding(m_image);
             } else {
                 // Final attempt: Search for the logo in resources
                 QDirIterator it(":", QDirIterator::Subdirectories);
                 while (it.hasNext()) {
                     QString found = it.next();
                     if (found.endsWith("/logo.png") && m_image.load(found)) {
                         applyRounding(m_image);
                         break;
                     }
                 }
             }
        }
    } else if (m_id.startsWith(":/")) {
        if (m_image.load(m_id)) {
             applyRounding(m_image);
        } else {
             qWarning() << "Failed to load resource icon:" << m_id;
        }
    }
    
    // 2. Try system theme
    if (m_image.isNull()) {
        QIcon icon = QIcon::fromTheme(m_id);
        if (!icon.isNull()) {
            QPixmap pix = icon.pixmap(size, size);
            if (!pix.isNull()) {
                m_image = pix.toImage();
                applyRounding(m_image);
            }
        }
    }

    // 3. Fallback to placeholder if still null
    if (m_image.isNull()) {
        m_image = QImage(size, size, QImage::Format_ARGB32);
        m_image.fill(Qt::transparent);
        
        QPainter p(&m_image);
        p.setRenderHint(QPainter::Antialiasing);

        // Pick color from string hash
        quint32 hash_color = qHash(m_id);
        QColor bg = QColor::fromHsl((hash_color % 360), 200, 150);
        
        // Draw rounded rect
        p.setBrush(bg);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(0, 0, size, size, size/4.0, size/4.0);
        
        // Draw initial
        p.setPen(Qt::white);
        QFont f = p.font();
        f.setPixelSize(size * 0.5);
        f.setBold(true);
        p.setFont(f);
        
        QString letter = m_id.left(1).toUpper();
        p.drawText(m_image.rect(), Qt::AlignCenter, letter);
    }
    
    // Save to cache
    m_image.save(cachePath, "PNG");
    
    emit finished();
}

QQuickTextureFactory *IconResponse::textureFactory() const
{
    return QQuickTextureFactory::textureFactoryForImage(m_image);
}

QQuickImageResponse *IconProvider::requestImageResponse(const QString &id, const QSize &requestedSize)
{
    auto response = new IconResponse(id);
    auto runner = new IconRunner(response);
    // Auto-delete runner, but Response is owned by QML engine
    runner->setAutoDelete(true); 
    QThreadPool::globalInstance()->start(runner);
    return response;
}

