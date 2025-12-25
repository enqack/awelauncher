#pragma once

#include <QQuickAsyncImageProvider>
#include <QQuickImageResponse>
#include <QQuickTextureFactory>
#include <QThreadPool>
#include <QImage>
#include <QString>

/**
 * @file IconProvider.h
 * @brief Asynchronous icon loading for QML Image elements.
 */

/**
 * @class IconResponse
 * @brief Handles the actual loading of an icon on a background thread.
 * 
 * This class is managed by the @c IconProvider and utilizes @c QThreadPool 
 * to fetch icons without blocking the main UI thread.
 */
class IconResponse : public QQuickImageResponse
{
public:
    IconResponse(const QString &id);
    /** @brief Returns the loaded texture to QML. */
    QQuickTextureFactory *textureFactory() const override;

    /** @brief Entry point for the threaded worker. */
    void run();

private:
    QString m_id;
    QImage m_image;
};

/**
 * @class IconProvider
 * @brief Registered with the QML engine as "icon" to provide images via "image://icon/<key>".
 * 
 * It returns an @c IconResponse immediately, which then loads the icon asynchronously.
 */
class IconProvider : public QQuickAsyncImageProvider
{
public:
    /** @brief Factory method called by QML engine. */
    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;
};
