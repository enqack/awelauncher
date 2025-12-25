#pragma once

#include <QObject>
#include <QMap>
#include <QString>

class MRUTracker : public QObject
{
    Q_OBJECT
public:
    static MRUTracker& instance();
    
    void recordActivation(const QString& itemId);
    int getBoost(const QString& itemId) const;
    
private:
    explicit MRUTracker(QObject *parent = nullptr);
    
    void load();
    void save();
    
    QMap<QString, qint64> m_lastUsed; // itemId -> timestamp (ms since epoch)
};
