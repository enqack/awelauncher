#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <vector>

struct LauncherItem {
    QString id;
    QString primary;
    QString secondary;
    QString exec;
    QString iconKey;
    bool selected = false;
    bool terminal = false;
    QVector<int> matchPositions;  // Indices of matched chars in primary text
};

class LauncherModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        PrimaryRole,
        SecondaryRole,
        IconKeyRole,
        SelectedRole,
        TerminalRole,
        ExecRole,
        MatchPositionsRole
    };

    explicit LauncherModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(const std::vector<LauncherItem>& items);
    
    Q_INVOKABLE void filter(const QString& query);
    
    void setShowMode(const QString& mode) { m_showMode = mode; }

private:
    std::vector<LauncherItem> m_allItems;
    std::vector<LauncherItem> m_displayedItems;
    QString m_showMode = "drun";
};
