#include "LauncherModel.h"
#include "../providers/DesktopFileLoader.h"
#include "../utils/FuzzyMatcher.h"
#include "../utils/MRUTracker.h"
#include <algorithm>

LauncherModel::LauncherModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // Items are now loaded in main.cpp to support providers/overrides better
}

int LauncherModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_displayedItems.size());
}

QVariant LauncherModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_displayedItems.size()))
        return QVariant();

    const auto &item = m_displayedItems[index.row()];

    switch (role) {
    case IdRole: return item.id;
    case PrimaryRole: return item.primary;
    case SecondaryRole: return item.secondary;
    case IconKeyRole: return item.iconKey;
    case SelectedRole: return item.selected;
    case TerminalRole: return item.terminal;
    case ExecRole: return item.exec;
    case MatchPositionsRole: return QVariant::fromValue(item.matchPositions);
    default: return QVariant();
    }
}

QHash<int, QByteArray> LauncherModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[PrimaryRole] = "primary";
    roles[SecondaryRole] = "secondary";
    roles[IconKeyRole] = "iconKey";
    roles[SelectedRole] = "selected";
    roles[TerminalRole] = "terminal";
    roles[ExecRole] = "exec";
    roles[MatchPositionsRole] = "matchPositions";
    return roles;
}

void LauncherModel::setItems(const std::vector<LauncherItem>& items)
{
    qDebug() << "LauncherModel::setItems called with" << items.size() << "items";
    beginResetModel();
    m_allItems = items;
    m_displayedItems = items;
    endResetModel();
    emit countChanged();
    qDebug() << "LauncherModel::setItems finished. Display count:" << m_displayedItems.size();
}

void LauncherModel::filter(const QString& query)
{
    qDebug() << "LauncherModel::filter called with:" << query << "Total Items:" << m_allItems.size();
    beginResetModel();
    if (query.isEmpty()) {
        // Show all items when empty (both drun and run modes)
        m_displayedItems = m_allItems;
    } else {
        // Fuzzy match and score all items
        struct ScoredItem {
            LauncherItem item;
            int score;
        };
        
        std::vector<ScoredItem> scoredItems;
        
        for (const auto& item : m_allItems) {
            // BUG FIX: Filter logic was skipping everything in 'run' mode incorrectly
            // If in 'run' mode, we only want to skip if the item isn't a 'path' or 'run' item
            // For now, let's keep it simple: just match everything in all modes.
            
            // Try matching against primary, secondary, and id
            auto primaryMatch = FuzzyMatcher::match(query, item.primary);
            auto secondaryMatch = FuzzyMatcher::match(query, item.secondary);
            auto idMatch = FuzzyMatcher::match(query, item.id);
            
            int bestScore = std::max({primaryMatch.score, secondaryMatch.score, idMatch.score});
            
            if (bestScore > 0) {
                LauncherItem itemWithPositions = item;
                // Store positions from the best match (primary takes precedence)
                if (primaryMatch.score == bestScore) {
                    itemWithPositions.matchPositions = primaryMatch.positions;
                } else if (secondaryMatch.score == bestScore) {
                    itemWithPositions.matchPositions = secondaryMatch.positions;
                } else {
                    itemWithPositions.matchPositions = idMatch.positions;
                }
                
                // Apply MRU boost
                int mruBoost = MRUTracker::instance().getBoost(item.id);
                int finalScore = bestScore + mruBoost;
                
                scoredItems.push_back({itemWithPositions, finalScore});
            }
        }
        
        // Sort by score descending
        std::sort(scoredItems.begin(), scoredItems.end(), 
                  [](const ScoredItem& a, const ScoredItem& b) {
                      return a.score > b.score;
                  });
        
        // Extract sorted items
        m_displayedItems.clear();
        for (const auto& scored : scoredItems) {
            m_displayedItems.push_back(scored.item);
        }
        
        // Fallback: simple "Run command" if no matches
        if (m_displayedItems.empty() && !query.trimmed().isEmpty() && m_fallbackEnabled) {
            LauncherItem runItem;
            runItem.id = "fallback:" + query;
            runItem.primary = "Run '" + query + "' in terminal";
            runItem.secondary = "Custom Command";
            runItem.iconKey = "utilities-terminal";
            
            // Just pass the query as exec. The Controller's logic for terminal vs shell 
            // depends on "TerminalRole". We want this to run in terminal usually?
            // "run in terminal" implies TerminalRole = true.
            runItem.exec = query; 
            runItem.terminal = true; 
            
            runItem.selected = false;
            m_displayedItems.push_back(runItem);
        }
    }
    endResetModel();
    emit countChanged();
}

