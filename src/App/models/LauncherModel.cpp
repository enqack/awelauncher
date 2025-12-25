#include "LauncherModel.h"
#include "../providers/DesktopFileLoader.h"
#include "../utils/FuzzyMatcher.h"
#include "../utils/MRUTracker.h"
#include <algorithm>

LauncherModel::LauncherModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // Load real data
    m_allItems = DesktopFileLoader::scan();
    m_displayedItems = m_allItems;
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
    beginResetModel();
    m_allItems = items;
    m_displayedItems = items;
    endResetModel();
}

void LauncherModel::filter(const QString& query)
{
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
            // Skip drun items if we're in run mode
            if (m_showMode == "run") {
                continue; // Don't match against desktop apps in run mode
            }
            
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
        
        // Run provider: if no matches and query looks like a command, add synthetic "Run" item
        if (m_displayedItems.empty() && !query.trimmed().isEmpty()) {
            LauncherItem runItem;
            runItem.id = "run:" + query;
            runItem.primary = "Run: " + query;
            runItem.secondary = "Execute as shell command";
            runItem.iconKey = "system-run";
            runItem.exec = query;
            runItem.terminal = false;
            runItem.selected = false;
            m_displayedItems.push_back(runItem);
        }
    }
    endResetModel();
}

