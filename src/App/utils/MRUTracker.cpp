#include "MRUTracker.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>

MRUTracker& MRUTracker::instance()
{
    static MRUTracker instance;
    return instance;
}

MRUTracker::MRUTracker(QObject *parent)
    : QObject(parent)
{
    load();
}

void MRUTracker::recordActivation(const QString& itemId)
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    m_lastUsed[itemId] = now;
    save();
}

int MRUTracker::getBoost(const QString& itemId) const
{
    if (!m_lastUsed.contains(itemId)) {
        return 0;
    }
    
    qint64 lastUsed = m_lastUsed[itemId];
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 ageMs = now - lastUsed;
    
    // Boost algorithm based on recency
    if (ageMs < 3600000) {  // < 1 hour
        return 500;
    } else if (ageMs < 86400000) {  // < 1 day
        return 200;
    } else if (ageMs < 604800000) {  // < 1 week
        return 50;
    }
    return 0;
}

void MRUTracker::load()
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QString mruPath = cacheDir + "/awelauncher/mru.json";
    
    QFile file(mruPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return; // No MRU file yet, that's fine
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return;
    }
    
    QJsonObject obj = doc.object();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        m_lastUsed[it.key()] = it.value().toVariant().toLongLong();
    }
}

void MRUTracker::save()
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QString cachePath = cacheDir + "/awelauncher";
    
    QDir dir;
    dir.mkpath(cachePath);
    
    QString mruPath = cachePath + "/mru.json";
    
    QJsonObject obj;
    for (auto it = m_lastUsed.begin(); it != m_lastUsed.end(); ++it) {
        obj[it.key()] = it.value();
    }
    
    QJsonDocument doc(obj);
    
    QFile file(mruPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to save MRU data to" << mruPath;
        return;
    }
    
    file.write(doc.toJson());
    file.close();
}
