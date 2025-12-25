#pragma once

#include <QString>
#include <QVector>

class FuzzyMatcher
{
public:
    struct MatchResult {
        int score = 0;
        QVector<int> positions;
        bool matched = false;
    };

    static MatchResult match(const QString& query, const QString& target);
    
private:
    static int scoreMatch(const QString& query, const QString& target, QVector<int>& positions);
};
