#include "StdinProvider.h"
#include <unistd.h>
#include <cstdio>
#include <QDebug>
#include <QSocketNotifier>
#include <QFile>

StdinProvider::StdinProvider(QObject *parent)
    : QObject(parent)
{
}

void StdinProvider::start()
{
    // Use QSocketNotifier to read stdin (fd 0) asynchronously
    m_notifier = new QSocketNotifier(STDIN_FILENO, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, &StdinProvider::onActivated);
    qDebug() << "Listening on stdin for items...";
}

void StdinProvider::onActivated(int socket)
{
    // Use C-style file reading or QFile wrapped around the descriptor.
    // Since we are inside a Qt loop, standard blocked read might be weird if notifier fires.
    // But notifier fires when data IS available.
    
    // We can use QFile with handle 0?
    QFile stdinFile;
    if (stdinFile.open(stdin, QIODevice::ReadOnly)) {
        while (!stdinFile.atEnd()) {
             QByteArray line = stdinFile.readLine();
             if (line.isEmpty()) break;
             
             QString text = QString::fromUtf8(line).trimmed();
             if (text.isEmpty()) continue;
             
             LauncherItem item;
             item.id = text;
             item.primary = text;
             item.secondary = "";
             item.iconKey = "application-x-executable"; // generic icon
             item.exec = text; // Just print the text back
             item.terminal = false;
             
             m_items.append(item);
        }
    }
    
    // Note: QFile(stdin) usually consumes everything available. 
    // If it blocks waiting for more, the GUI freezes.
    // A safer low-level read is preferred if QFile behaviors are tricky with pipes.
    // However, for this simple implementation, let's assume line-based buffering.
    
    emit itemsChanged();
}

QVector<LauncherItem> StdinProvider::getItems() const
{
    return m_items;
}
