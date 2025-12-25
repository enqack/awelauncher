#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <vector>

/**
 * @struct LauncherItem
 * @brief Represents a single entry in the launcher (App, Command, or Window).
 */
struct LauncherItem {
    QString id;                 /**< Unique identifier */
    QString primary;            /**< Main text (App name, Window title) */
    QString secondary;          /**< Subtext (Description, app_id) */
    QString exec;               /**< Command to execute (if applicable) */
    QString iconKey;            /**< Icon name or path */
    bool selected = false;      /**< Selection state */
    bool terminal = false;      /**< Whether to run in terminal */
    QVector<int> matchPositions; /**< Indices of characters matched by fuzzy filter */
};

/**
 * @class LauncherModel
 * @brief Qt ListModel providing data to the QML results list.
 */
class LauncherModel : public QAbstractListModel
{
    Q_OBJECT
public:
    /**
     * @brief Data roles for QML bindings.
     */
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

    /** @brief Populates the model with a new set of items. */
    void setItems(const std::vector<LauncherItem>& items);
    
    /** @brief Filters the internal item list based on a query string. */
    Q_INVOKABLE void filter(const QString& query);
    
    /** @brief Sets the provider mode (drun, run, window). */
    void setShowMode(const QString& mode) { m_showMode = mode; }

private:
    std::vector<LauncherItem> m_allItems;
    std::vector<LauncherItem> m_displayedItems;
    QString m_showMode = "drun";
};
