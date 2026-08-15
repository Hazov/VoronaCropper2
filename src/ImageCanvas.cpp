#include "ImageCanvas.h"

#include <QPainter>
#include <QPaintEvent>

ImageCanvas::ImageCanvas(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(400, 300);
}

void ImageCanvas::setImage(const QImage &newImage)
{
    image = newImage;
    update();
}

void ImageCanvas::clearImage()
{
    image = QImage();
    update();
}

void ImageCanvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    // Фон области изображения
    painter.fillRect(rect(), Qt::color0);

    // Если изображения нет — показываем подсказку
    if (image.isNull())
    {
        painter.setPen(Qt::darkGray);

        QFont font = painter.font();
        font.setPointSize(14);
        painter.setFont(font);
        

        painter.drawText(
            rect(),
            Qt::AlignCenter,
            "Перетащите фотографии сюда"
        );

        return;
    }

    // Масштабируем фотографию с сохранением пропорций
    const QSize scaledSize =
        image.size().scaled(
            size(),
            Qt::KeepAspectRatio
        );

    const int x =
        (width() - scaledSize.width()) / 2;

    const int y =
        (height() - scaledSize.height()) / 2;

    const QRect targetRect(
        x,
        y,
        scaledSize.width(),
        scaledSize.height()
    );

    painter.drawImage(
        targetRect,
        image
    );
}