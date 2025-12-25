#pragma once

#include <QString>
#include <QVector>

/**
 * @class FuzzyMatcher
 * @brief Implements a fuzzy matching algorithm for search filtering.
 * 
 * The algorithm provides scoring based on:
 * - Sequential matches (+100 per char)
 * - Start-of-string bonus (+50)
 * - Consecutive character bonus (+20 per char)
 * - Exact case bonus (+10)
 * - Late match penalty (-1 per target index)
 */
class FuzzyMatcher
{
public:
    /** @brief Holds the results of a single fuzzy match attempt. */
    struct MatchResult {
        int score = 0;          /**< Total calculated score (higher is better) */
        QVector<int> positions; /**< Indices of the characters that matched */
        bool matched = false;   /**< True if the entire query was found in order */
    };

    /** @brief Executes a fuzzy match of @p query against @p target. */
    static MatchResult match(const QString& query, const QString& target);
    
private:
    static int scoreMatch(const QString& query, const QString& target, QVector<int>& positions);
};
