#include "MRUTracker.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
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
    
    // Create or update entry
    HistoryEntry& entry = m_history[itemId];
    entry.id = itemId;
    entry.lastUsed = now;
    entry.count++;
    
    save();
}

int MRUTracker::getBoost(const QString& itemId) const
{
    if (!m_history.contains(itemId)) {
        return 0;
    }
    
    const HistoryEntry& entry = m_history[itemId];
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 ageMs = now - entry.lastUsed;
    
    int score = 0;
    
    // 1. Recency Boost
    if (ageMs < 3600000) {  // < 1 hour
        score += 500;
    } else if (ageMs < 86400000) {  // < 1 day
        score += 200;
    } else if (ageMs < 604800000) {  // < 1 week
        score += 50;
    }
    
    // 2. Frequency Boost (cap at 500)
    int freqBoost = std::min(entry.count * 10, 500);
    score += freqBoost;
    
    return score;
}

void MRUTracker::load()
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QString historyPath = cacheDir + "/awelauncher/history.json";
    
    // Migration: Check for old mru.json if history.json doesn't exist
    if (!QFile::exists(historyPath)) {
        QString oldMruPath = cacheDir + "/awelauncher/mru.json";
        if (QFile::exists(oldMruPath)) {
            qInfo() << "[MRU] Migrating from mru.json to history.json";
            QFile oldFile(oldMruPath);
            if (oldFile.open(QIODevice::ReadOnly)) {
                 QJsonDocument doc = QJsonDocument::fromJson(oldFile.readAll());
                 if (doc.isObject()) {
                     QJsonObject obj = doc.object();
                     for (auto it = obj.begin(); it != obj.end(); ++it) {
                         HistoryEntry e;
                         e.id = it.key();
                         e.lastUsed = it.value().toVariant().toLongLong();
                         e.count = 1; // Start with count 1
                         m_history[e.id] = e;
                     }
                 }
                 oldFile.close();
            }
            save(); // Save immediately in new format
            return;
        }
    }
    
    QFile file(historyPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (doc.isObject() && doc.object()["history"].isArray()) {
        QJsonArray arr = doc.object()["history"].toArray();
        for (const auto& val : arr) {
            QJsonObject item = val.toObject();
            HistoryEntry e;
            e.id = item["id"].toString();
            e.count = item["count"].toInt();
            e.lastUsed = static_cast<qint64>(item["last"].toDouble());
            m_history[e.id] = e;
        }
    }
}

void MRUTracker::save()
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QString cachePath = cacheDir + "/awelauncher";
    QDir dir;
    dir.mkpath(cachePath);
    
    QString historyPath = cachePath + "/history.json";
    
    QJsonArray arr;
    for (auto it = m_history.begin(); it != m_history.end(); ++it) {
        QJsonObject item;
        item["id"] = it.value().id;
        item["count"] = it.value().count;
        item["last"] = static_cast<double>(it.value().lastUsed);
        arr.append(item);
    }
    
    QJsonObject root;
    root["history"] = arr;
    
    QJsonDocument doc(root);
    
    QFile file(historyPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to save history data to" << historyPath;
        return;
    }
    
    file.write(doc.toJson());
    file.close();
}
