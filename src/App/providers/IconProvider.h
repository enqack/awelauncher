#pragma once

#include <QQuickAsyncImageProvider>
#include <QThreadPool>

class IconResponse : public QQuickImageResponse
{
public:
    IconResponse(const QString &id);
    QQuickTextureFactory *textureFactory() const override;

    void run();

private:
    QString m_id;
    QImage m_image;
};


class IconProvider : public QQuickAsyncImageProvider
{
public:
    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;
};
