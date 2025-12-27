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
    
    struct HistoryEntry {
        QString id;
        int count = 0;
        qint64 lastUsed = 0;
    };

    QMap<QString, HistoryEntry> m_history; 
};
