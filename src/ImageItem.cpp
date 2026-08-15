#include "ImageItem.h"
#include <QPainter>
#include <QDebug>
#include <algorithm>

ImageItem::ImageItem(const QString &filePath)
    : m_filePath(filePath),
      m_originalImage(filePath)
{
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

