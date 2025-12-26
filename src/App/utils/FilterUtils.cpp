#include "FilterUtils.h"
#include <QRegularExpression>

bool FilterUtils::matches(const QString& text, const QStringList& rules)
{
    for (const auto& rule : rules) {
        if (rule.startsWith("/") && rule.endsWith("/")) {
            // Regex
            QString pattern = rule.mid(1, rule.length()-2);
            if (QRegularExpression(pattern).match(text).hasMatch()) return true;
        } else {
            // Substring
            if (text.contains(rule, Qt::CaseInsensitive)) return true;
        }
    }
    return false;
}
