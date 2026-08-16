#include "ImageItem.h"

#include <QPainter>
#include <QDebug>
#include <QFileInfo>

#include <algorithm>
#include <cstring>

#include <libheif/heif.h>


ImageItem::ImageItem(const QString &filePath)
    : m_filePath(filePath)
{
    m_originalImage =
        loadImage(filePath);

    if (!m_originalImage.isNull())
    {
        m_cropCenterX =
            m_originalImage.width() / 2.0;

        m_cropCenterY =
            m_originalImage.height() / 2.0;

        if (m_originalImage.width() >
            m_originalImage.height())
        {
            m_cropWidthCm = 15.0;
            m_cropHeightCm = 10.0;
        }
    }
}

QImage ImageItem::loadImage(
    const QString& filePath
) const
{
    const QString suffix =
        QFileInfo(filePath)
            .suffix()
            .toLower();

    qDebug() << "FILE:"
             << filePath;

    qDebug() << "SUFFIX:"
             << suffix;


    if (suffix == "heic" ||
        suffix == "heif")
    {
        qDebug() << "HEIC branch";

        return loadHeicImage(filePath);
    }

    return QImage(filePath);
}

QImage ImageItem::loadHeicImage(
    const QString& filePath
) const
{
    heif_context* context =
        heif_context_alloc();

    if (!context)
    {
        qDebug()
            << "HEIC: не удалось создать heif_context";

        return QImage();
    }


    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug()
            << "HEIC: не удалось открыть файл:"
            << filePath;

        heif_context_free(context);

        return QImage();
    }


    QByteArray fileData =
        file.readAll();

    file.close();


    heif_error error =
        heif_context_read_from_memory_without_copy(
            context,
            fileData.constData(),
            fileData.size(),
            nullptr
        );


    if (error.code != heif_error_Ok)
    {
        qDebug()
            << "HEIC: ошибка чтения:"
            << error.message;

        heif_context_free(context);

        return QImage();
    }


    const int count =
        heif_context_get_number_of_top_level_images(
            context
        );

    qDebug()
        << "Количество изображений HEIC:"
        << count;


    heif_image_handle* handle = nullptr;


    error =
        heif_context_get_primary_image_handle(
            context,
            &handle
        );


    if (error.code != heif_error_Ok)
    {
        qDebug()
            << "HEIC: не удалось получить primary image:"
            << error.message;

        heif_context_free(context);

        return QImage();
    }


    const int width =
        heif_image_handle_get_width(handle);

    const int height =
        heif_image_handle_get_height(handle);


    heif_image* image = nullptr;


    error =
        heif_decode_image(
            handle,
            &image,
            heif_colorspace_RGB,
            heif_chroma_interleaved_RGBA,
            nullptr
        );


    if (error.code != heif_error_Ok)
    {
        qDebug()
            << "HEIC: ошибка декодирования:"
            << error.message;

        heif_image_handle_release(handle);
        heif_context_free(context);

        return QImage();
    }


    int stride = 0;


    const uint8_t* data =
        heif_image_get_plane_readonly(
            image,
            heif_channel_interleaved,
            &stride
        );


    if (!data)
    {
        qDebug()
            << "HEIC: не удалось получить данные изображения";

        heif_image_release(image);
        heif_image_handle_release(handle);
        heif_context_free(context);

        return QImage();
    }


    QImage result(
        width,
        height,
        QImage::Format_RGBA8888
    );


    for (int y = 0; y < height; ++y)
    {
        std::memcpy(
            result.scanLine(y),
            data + y * stride,
            width * 4
        );
    }


    heif_image_release(image);
    heif_image_handle_release(handle);
    heif_context_free(context);


    return result;
}

bool ImageItem::isValid() const
{
    return !m_originalImage.isNull();
}

const QImage &ImageItem::image() const
{
    return m_originalImage;
}

const QImage &ImageItem::originalImage() const
{
    return m_originalImage;
}

const QImage &ImageItem::croppedImage() const
{
    return m_croppedImage;
}

const QString &ImageItem::filePath() const
{
    return m_filePath;
}

double ImageItem::cropWidthCm() const
{
    return m_cropWidthCm;
}

double ImageItem::cropHeightCm() const
{
    return m_cropHeightCm;
}

double ImageItem::cropCenterX() const
{
    return m_cropCenterX;
}

double ImageItem::cropCenterY() const
{
    return m_cropCenterY;
}

void ImageItem::setCropSize(
    double widthCm,
    double heightCm)
{
    m_cropWidthCm = widthCm;
    m_cropHeightCm = heightCm;
}

void ImageItem::setCropCenter(
    double x,
    double y)
{
    m_cropCenterX = x;
    m_cropCenterY = y;
}

void ImageItem::createCroppedImage(
    const QRectF &cropRect)
{
    if (m_originalImage.isNull())
        return;

    const QRect cropBounds =
        cropRect.toAlignedRect();

    if (cropBounds.isEmpty())
    {
        m_croppedImage = QImage();
        return;
    }
    
    
    

    // Создаём результат полного размера рамки.
    // Области за пределами исходной фотографии
    // останутся белыми.
    m_croppedImage = QImage(
        cropBounds.size(),
        QImage::Format_RGB32
    );

    m_croppedImage.fill(Qt::white);

    const QRect imageBounds =
        m_originalImage.rect();

    // Какая часть рамки реально попадает
    // внутрь исходной фотографии.
    const QRect sourceRect =
        cropBounds.intersected(imageBounds);

    if (sourceRect.isEmpty())
        return;

    // Положение этой части относительно
    // верхнего левого угла результата.
    const QPoint destinationPosition =
        sourceRect.topLeft() -
        cropBounds.topLeft();

    const QRect destinationRect(
        destinationPosition,
        sourceRect.size()
    );

    QPainter painter(&m_croppedImage);

    painter.drawImage(
        destinationRect,
        m_originalImage,
        sourceRect
    );
}

void ImageItem::createCroppedImage()
{
    if (m_originalImage.isNull())
        return;

    const double cropWidth =
        m_cropWidthCm / 10.0 *
        m_originalImage.width();

    const double cropHeight =
        m_cropHeightCm / 15.0 *
        m_originalImage.height();

    const QRectF cropRect(
        m_cropCenterX - cropWidth / 2.0,
        m_cropCenterY - cropHeight / 2.0,
        cropWidth,
        cropHeight
    );

    createCroppedImage(cropRect);
}

QRectF ImageItem::cropRect() const
{
    return m_cropRect;
}

void ImageItem::setCropRect(
    const QRectF &rect)
{
    m_cropRect = rect;
}

int ImageItem::levelsStatus() const
{
    return m_levelsStatus;
}

int ImageItem::curvesStatus() const
{
    return m_curvesStatus;
}

void ImageItem::setLevelsStatus(int value)
{
    m_levelsStatus =
        std::clamp(
            value,
            MIN_LIGHT_STATUS,
            MAX_LIGHT_STATUS
        );
}

void ImageItem::setCurvesStatus(int value)
{
    m_curvesStatus =
        std::clamp(
            value,
            MIN_LIGHT_STATUS,
            MAX_LIGHT_STATUS
        );
}

void ImageItem::setFitImageMode(
    bool enabled,
    bool withMargins
)
{
    m_fitImageMode = enabled;
    m_fitImageWithMargins = withMargins;
}

bool ImageItem::fitImageMode() const
{
    return m_fitImageMode;
}

bool ImageItem::fitImageWithMargins() const
{
    return m_fitImageWithMargins;
}

void ImageItem::createFittedCroppedImage(
    const QRectF& cropRect,
    bool withMargins
)
{
    if (m_originalImage.isNull())
        return;

    const int outputWidth =
        qRound(cropRect.width());

    const int outputHeight =
        qRound(cropRect.height());

    if (outputWidth <= 0 ||
        outputHeight <= 0)
    {
        return;
    }

    QImage result(
        outputWidth,
        outputHeight,
        QImage::Format_ARGB32
    );

    result.fill(Qt::white);

    QRect targetRect(
        0,
        0,
        outputWidth,
        outputHeight
    );

    if (withMargins)
    {
        const double marginX =
            outputWidth *
            (0.4 / m_cropWidthCm);

        const double marginY =
            outputHeight *
            (0.4 / m_cropHeightCm);

        targetRect.adjust(
            qRound(marginX),
            qRound(marginY),
            -qRound(marginX),
            -qRound(marginY)
        );
    }

    const QSize fittedSize =
        m_originalImage.size()
            .scaled(
                targetRect.size(),
                Qt::KeepAspectRatio
            );

    const int x =
        targetRect.x()
        + (
            targetRect.width()
            - fittedSize.width()
        ) / 2;

    const int y =
        targetRect.y()
        + (
            targetRect.height()
            - fittedSize.height()
        ) / 2;

    const QRect imageTarget(
        x,
        y,
        fittedSize.width(),
        fittedSize.height()
    );

    QPainter painter(&result);

    painter.drawImage(
        imageTarget,
        m_originalImage
    );

    painter.end();

    m_croppedImage = result;
}

void ImageItem::setCroppedImage(
    const QImage& image
)
{
    m_croppedImage = image;
}

bool ImageItem::isMarked() const
{
    return m_marked;
}

void ImageItem::setMarked(bool marked)
{
    m_marked = marked;
}