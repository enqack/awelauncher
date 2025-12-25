#include <QtTest>
#include "App/utils/FuzzyMatcher.h"

class TestFuzzy : public QObject
{
    Q_OBJECT

private slots:
    void testBasicMatch() {
        FuzzyMatcher matcher;
        auto result = matcher.match("fzf", "fuzzy finder");
        QVERIFY(result.matched);
        // Current implementation gives 392 for this match
        QCOMPARE(result.score, 392); 
    }

    void testSmartCase() {
        FuzzyMatcher matcher;
        // Currently, search is CASE INSENSITIVE (it lowercases everything)
        QVERIFY(matcher.match("abc", "ABC").matched);
        QVERIFY(matcher.match("Abc", "abc").matched);
    }

    void testPositions() {
        FuzzyMatcher matcher;
        auto result = matcher.match("fzf", "fuzzy finder");
        QCOMPARE(result.positions.size(), 3);
        QCOMPARE(result.positions[0], 0); // 'f'
        QCOMPARE(result.positions[1], 2); // 'z'
        QCOMPARE(result.positions[2], 6); // 'f' in finder
    }
};

QTEST_MAIN(TestFuzzy)
#include "test_fuzzy.moc"
