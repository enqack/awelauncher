#include "IconProvider.h"
#include <QRunnable>
#include <QPainter>
#include <QThread>
#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>

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
    
    // Create cache key from id + size
    QString cacheKey = QString("%1_%2").arg(m_id).arg(size);
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

