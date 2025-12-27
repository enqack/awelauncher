#include "FuzzyMatcher.h"
#include <QChar>

FuzzyMatcher::MatchResult FuzzyMatcher::match(const QString& query, const QString& target)
{
    MatchResult result;
    
    if (query.isEmpty()) {
        result.matched = true;
        result.score = 0;
        return result;
    }

    // Tier 1: Exact Match
    if (query.compare(target, Qt::CaseInsensitive) == 0) {
        result.matched = true;
        result.isExact = true;
        result.score = 10000;
        for(int i=0; i<query.length(); ++i) result.positions.push_back(i);
        return result;
    }

    // Tier 2: Prefix Match
    if (target.startsWith(query, Qt::CaseInsensitive)) {
        result.matched = true;
        result.isPrefix = true;
        // Base 5000 + bonus for length to break ties
        result.score = 5000 + (query.length() * 10);
        for(int i=0; i<query.length(); ++i) result.positions.push_back(i);
        return result;
    }
    
    result.score = scoreMatch(query, target, result.positions);
    result.matched = (result.score > 0);
    
    return result;
}

int FuzzyMatcher::scoreMatch(const QString& query, const QString& target, QVector<int>& positions)
{
    QString lowerQuery = query.toLower();
    QString lowerTarget = target.toLower();
    
    int queryLen = lowerQuery.length();
    int targetLen = lowerTarget.length();
    
    if (queryLen > targetLen) return 0;
    
    // Try to find all query chars in order
    int queryIdx = 0;
    int score = 0;
    int consecutiveBonus = 0;
    int lastMatchPos = -1;
    
    positions.clear();
    
    for (int targetIdx = 0; targetIdx < targetLen && queryIdx < queryLen; ++targetIdx) {
        if (lowerQuery[queryIdx] == lowerTarget[targetIdx]) {
            positions.append(targetIdx);
            
            // Base score
            score += 100;
            
            // Bonus for match at start
            if (targetIdx == 0) score += 50;
            
            // Tier 3: Bonus for Word Boundary (Acronyms)
            bool isWordStart = (targetIdx == 0) || 
                               !target[targetIdx-1].isLetterOrNumber() ||
                               (target[targetIdx].isUpper() && !target[targetIdx-1].isUpper());
            
            if (isWordStart) {
                score += 100; // Acronym bonus
            }

            // Bonus for consecutive matches
            if (lastMatchPos == targetIdx - 1) {
                consecutiveBonus += 20;
                score += consecutiveBonus;
            } else {
                consecutiveBonus = 0;
            }
            
            // Bonus for exact case match
            if (query[queryIdx] == target[targetIdx]) {
                score += 10;
            }
            
            // Penalty for late matches
            score -= targetIdx;
            
            lastMatchPos = targetIdx;
            queryIdx++;
        }
    }
    
    // If we didn't match all query chars, no match
    if (queryIdx != queryLen) {
        positions.clear();
        return 0;
    }
    
    return score;
}
