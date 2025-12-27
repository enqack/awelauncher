#pragma once

#include <cstdio>
#include <QString>
#include <QElapsedTimer>
#include "Config.h"

// Shared profiling macro
#define APP_PROFILE_POINT(timer, name) \
    if (Config::instance().isDebug()) \
        fprintf(stderr, "[PROFILE] %s : %lld ms\n", qPrintable(QString(name)), timer.elapsed());
