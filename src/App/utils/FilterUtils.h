#pragma once

#include <QString>
#include <QStringList>

class FilterUtils {
public:
    static bool matches(const QString& text, const QStringList& rules);
};
