#include "CropCanvas.h"

#include <QPainter>
#include <QKeyEvent>

#include <algorithm>
#include <QPainterPath>
#include <QDebug>

CropCanvas::CropCanvas(QWidget* parent)
    : QWidget(parent),
      m_cropWidth(10.0),
      m_cropHeight(15.0),
      m_cropCenterX(0.0),
      m_cropCenterY(0.0),
      m_cropPixelWidth(0.0),
      m_cropPixelHeight(0.0),
      m_overflowLeft(0.0),
      m_overflowRight(0.0),
      m_overflowTop(0.0),
      m_overflowBottom(0.0),
      m_moveTimer(new QTimer(this)),
      m_leftPressed(false),
      m_rightPressed(false),
      m_upPressed(false),
      m_downPressed(false),
      m_allowOverflowLeft(false),
      m_allowOverflowRight(false),
      m_allowOverflowTop(false),
      m_allowOverflowBottom(false)
{
    setCursor(Qt::PointingHandCursor);

    setFocusPolicy(Qt::StrongFocus);

    m_moveTimer->setInterval(30);

    connect(
        m_moveTimer,
        &QTimer::timeout,
        this,
        &CropCanvas::moveCrop
    );

    setMinimumSize(400, 300);
    setAutoFillBackground(false);
    setMouseTracking(true);
}

void CropCanvas::setImage(const QImage& image)
{
    setCursor(Qt::ArrowCursor);

    m_image = image;

    updateScaledImage();

    resetCrop();

    setFocus();
    update();
}


void CropCanvas::clearImage()
{
    setCursor(Qt::PointingHandCursor);

    m_image = QImage();
    m_scaledImage = QImage();

    m_showCropResult = false;
    m_fitImageMode = false;

    m_mouseMode = MouseMode::None;
    m_resizeHandle = ResizeHandle::None;

    update();
}

void CropCanvas::setCropRatio(
    double width,
    double height
)
{
    if (width <= 0.0 || height <= 0.0)
        return;

    m_cropWidthCm = width;
    m_cropHeightCm = height;

    m_cropWidth = width;
    m_cropHeight = height;

    m_overflowLeft = 0.0;
    m_overflowRight = 0.0;
    m_overflowTop = 0.0;
    m_overflowBottom = 0.0;

    if (m_image.isNull())
    {
        update();
        return;
    }

    const double ratio =
        m_cropWidthCm / m_cropHeightCm;

    const double imageWidth =
        m_image.width();

    const double imageHeight =
        m_image.height();

    // Пересчитываем размер рамки,
    // сохраняя её текущий центр.
    if (imageWidth / imageHeight > ratio)
    {
        m_cropPixelHeight =
            imageHeight;

        m_cropPixelWidth =
            imageHeight * ratio;
    }
    else
    {
        m_cropPixelWidth =
            imageWidth;

        m_cropPixelHeight =
            imageWidth / ratio;
    }

    // После изменения размера рамки
    // всегда возвращаем её в центр изображения.
    m_cropCenterX =
        imageWidth / 2.0;

    m_cropCenterY =
        imageHeight / 2.0;

    // Старый выход за границу больше не актуален.
    m_allowOverflowLeft = false;
    m_allowOverflowRight = false;
    m_allowOverflowTop = false;
    m_allowOverflowBottom = false;

    m_overflowLeft = 0.0;
    m_overflowRight = 0.0;
    m_overflowTop = 0.0;
    m_overflowBottom = 0.0;

    update();

    update();
}

void CropCanvas::resetCrop()
{
    if (m_image.isNull())
        return;

    const double imageWidth =
        m_image.width();

    const double imageHeight =
        m_image.height();

    const double ratio =
        m_cropWidth / m_cropHeight;

    // Максимальная рамка с нужным соотношением,
    // помещающаяся целиком в изображение.
    if (imageWidth / imageHeight > ratio)
    {
        m_cropPixelHeight = imageHeight;
        m_cropPixelWidth =
            imageHeight * ratio;
    }
    else
    {
        m_cropPixelWidth = imageWidth;
        m_cropPixelHeight =
            imageWidth / ratio;
    }

    m_cropCenterX =
        imageWidth / 2.0;

    m_cropCenterY =
        imageHeight / 2.0;

    m_overflowLeft = 0.0;
    m_overflowRight = 0.0;
    m_overflowTop = 0.0;
    m_overflowBottom = 0.0;

    update();
}

QRectF CropCanvas::cropRectInImage() const
{
    return QRectF(
        m_cropCenterX - m_cropPixelWidth / 2.0,
        m_cropCenterY - m_cropPixelHeight / 2.0,
        m_cropPixelWidth,
        m_cropPixelHeight
    );
}

double CropCanvas::cropCenterX() const
{
    return m_cropCenterX;
}

double CropCanvas::cropCenterY() const
{
    return m_cropCenterY;
}

void CropCanvas::setCropCenter(double x, double y)
{
    if (m_image.isNull())
        return;

    m_cropCenterX = x;
    m_cropCenterY = y;

    update();
}

void CropCanvas::updateCropFrame() const
{
    if (m_image.isNull())
        return;

    // Здесь больше НЕ ограничиваем положение рамки
    // границами фотографии.
    //
    // Ограничение выполняется непосредственно в moveCrop(),
    // потому что рамка может законно выйти за пределы
    // фотографии максимум на 4 мм.
}

QRectF CropCanvas::imageRectOnScreen() const
{
    if (m_image.isNull())
        return QRectF();

    const QSizeF availableSize = size();

    const double imageWidth =
        m_image.width();

    const double imageHeight =
        m_image.height();

    // Технический запас относительно ФИЗИЧЕСКОГО
    // размера рамки, а не исходного изображения.
    const double overflowRatioX =
        (MAX_OVERFLOW_MM / 10.0) / m_cropWidthCm;

    const double overflowRatioY =
        (MAX_OVERFLOW_MM / 10.0) / m_cropHeightCm;

    // Оставляем запас с двух сторон.
    const double requiredWidth =
        imageWidth *
        (1.0 + 2.0 * overflowRatioX);

    const double requiredHeight =
        imageHeight *
        (1.0 + 2.0 * overflowRatioY);

    const double scale =
        std::min(
            availableSize.width() / requiredWidth,
            availableSize.height() / requiredHeight
        );

    const double width =
        imageWidth * scale;

    const double height =
        imageHeight * scale;

    const double x =
        (availableSize.width() - width) / 2.0;

    const double y =
        (availableSize.height() - height) / 2.0;

    return QRectF(
        x,
        y,
        width,
        height
    );
}

QRectF CropCanvas::cropRectOnScreen() const
{
    if (m_image.isNull())
        return QRectF();

    const QRectF imageRect =
        imageRectOnScreen();

    const double scaleX =
        imageRect.width() /
        m_image.width();

    const double scaleY =
        imageRect.height() /
        m_image.height();

    const double width =
        m_cropPixelWidth * scaleX;

    const double height =
        m_cropPixelHeight * scaleY;

    const double x =
        imageRect.left()
        + (
            m_cropCenterX
            - m_cropPixelWidth / 2.0
        ) * scaleX;

    const double y =
        imageRect.top()
        + (
            m_cropCenterY
            - m_cropPixelHeight / 2.0
        ) * scaleY;

    return QRectF(
        x,
        y,
        width,
        height
    );
}

void CropCanvas::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    painter.setRenderHint(
        QPainter::SmoothPixmapTransform,
        true
    );

    // =========================================================
    // Нет фотографии
    // =========================================================
    if (m_image.isNull())
    {

        // Тёмный текст
        painter.setPen(
            QColor("#333333")
        );

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

    // =========================================================
    // Фотография загружена
    // =========================================================

    // Серый фон вокруг фотографии
    painter.fillRect(
        rect(),
        QColor(128, 128, 128)
    );

    if (m_showCropResult)
    {
        painter.drawImage(
            imageRectOnScreen(),
            m_scaledImage
        );

        return;
    }

    const QRectF imageRect =
        imageRectOnScreen();

    const QRectF cropRect =
        cropRectOnScreen();

    const QRectF displayImageRect =
        displayImageRectOnScreen();

    if (m_fitImageMode)
    {
        // Белый фон внутри рамки.
        painter.fillRect(
            cropRectOnScreen(),
            Qt::white
        );
    }

    painter.drawImage(
        displayImageRect,
        m_scaledImage
    );

    if (m_image.size() != m_scaledImage.size())
    {
        // Ничего не делаем.
    }

    // =========================================================
    // Белая область внутри рамки за пределами фотографии
    // =========================================================

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);

    const double overflowScreenX =
        cropRect.width() *
        ((MAX_OVERFLOW_MM / 10.0) / m_cropWidthCm);

    const double overflowScreenY =
        cropRect.height() *
        ((MAX_OVERFLOW_MM / 10.0) / m_cropHeightCm);


    // ---------------------------------------------------------
    // СЛЕВА
    // ---------------------------------------------------------

    if (cropRect.left() < imageRect.left())
    {
        const double width =
            std::min(
                imageRect.left() - cropRect.left(),
                overflowScreenX
            );

        painter.drawRect(
            QRectF(
                cropRect.left(),
                cropRect.top(),
                width,
                cropRect.height()
            )
        );
    }


    // ---------------------------------------------------------
    // СПРАВА
    // ---------------------------------------------------------

    if (cropRect.right() > imageRect.right())
    {
        const double width =
            std::min(
                cropRect.right() - imageRect.right(),
                overflowScreenX
            );

        painter.drawRect(
            QRectF(
                cropRect.right() - width,
                cropRect.top(),
                width,
                cropRect.height()
            )
        );
    }


    // ---------------------------------------------------------
    // СВЕРХУ
    // ---------------------------------------------------------

    if (cropRect.top() < imageRect.top())
    {
        const double height =
            std::min(
                imageRect.top() - cropRect.top(),
                overflowScreenY
            );

        painter.drawRect(
            QRectF(
                cropRect.left(),
                cropRect.top(),
                cropRect.width(),
                height
            )
        );
    }


    // ---------------------------------------------------------
    // СНИЗУ
    // ---------------------------------------------------------

    if (cropRect.bottom() > imageRect.bottom())
    {
        const double height =
            std::min(
                cropRect.bottom() - imageRect.bottom(),
                overflowScreenY
            );

        painter.drawRect(
            QRectF(
                cropRect.left(),
                cropRect.bottom() - height,
                cropRect.width(),
                height
            )
        );
    }

    // =========================================================
    // Затемнение области за рамкой
    // =========================================================

    painter.setBrush(
        QColor(0, 0, 0, 120)
    );

    painter.setPen(Qt::NoPen);

    QPainterPath outsidePath;

    outsidePath.addRect(
        imageRect
    );

    QPainterPath cropPath;

    cropPath.addRect(
        cropRect
    );

    outsidePath =
        outsidePath.subtracted(
            cropPath
        );

    painter.drawPath(
        outsidePath
    );

    // =========================================================
    // Рамка
    // =========================================================

    painter.setBrush(
        Qt::NoBrush
    );

    QPen framePen(
        Qt::white
    );

    framePen.setWidth(2);

    painter.setPen(
        framePen
    );

    painter.drawRect(
        cropRect
    );

    // =========================================================
    // Сетка 3 × 3
    // =========================================================

    QPen gridPen(
        QColor(210, 210, 210)
    );

    gridPen.setWidth(1);

    painter.setPen(
        gridPen
    );

    const double thirdWidth =
        cropRect.width() / 3.0;

    const double thirdHeight =
        cropRect.height() / 3.0;

    painter.drawLine(
        cropRect.left() + thirdWidth,
        cropRect.top(),
        cropRect.left() + thirdWidth,
        cropRect.bottom()
    );

    painter.drawLine(
        cropRect.left() + thirdWidth * 2.0,
        cropRect.top(),
        cropRect.left() + thirdWidth * 2.0,
        cropRect.bottom()
    );

    painter.drawLine(
        cropRect.left(),
        cropRect.top() + thirdHeight,
        cropRect.right(),
        cropRect.top() + thirdHeight
    );

    painter.drawLine(
        cropRect.left(),
        cropRect.top() + thirdHeight * 2.0,
        cropRect.right(),
        cropRect.top() + thirdHeight * 2.0
    );
}

void CropCanvas::keyPressEvent(
    QKeyEvent* event
)
{
    if (m_image.isNull())
    {
        QWidget::keyPressEvent(event);
        return;
    }

    if (event->isAutoRepeat())
    {
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Left ||
    event->key() == Qt::Key_Right ||
    event->key() == Qt::Key_Up ||
    event->key() == Qt::Key_Down)
    {
        // По умолчанию считаем, что CropCanvas
        // должен обработать стрелку сам.
        event->setAccepted(false);

        emit keyPressed(event);

        // Если MainWindow обработал клавишу
        // (это происходит на стадии Light),
        // CropCanvas больше ничего не делает.
        if (event->isAccepted())
            return;
    }

    if (event->key() == Qt::Key_Escape)
    {
        if (m_fitImageMode)
        {
            exitFitImageMode();

            event->accept();
            return;
        }

        emit backRequested();

        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Backspace)
    {
        emit backRequested();

        event->accept();
        return;
    }

    // =========================================================
    // ENTER / SHIFT+ENTER
    // =========================================================

    // =========================================================
    // SHIFT — включение/выключение fit-режима
    // =========================================================

    if (event->key() == Qt::Key_Shift)
    {
        toggleFitImageMode();

        event->accept();
        return;
    }

    // =========================================================
    // ENTER — переход вперёд
    // =========================================================

    if (event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter)
    {
        emit forwardRequested();

        event->accept();
        return;
    }

    // =========================================================
    // В fit-режиме остальные клавиши не работают
    // =========================================================

    if (m_fitImageMode)
    {
        event->accept();
        return;
    }

    // =========================================================
    // ОСТАЛЬНЫЕ КЛАВИШИ
    // =========================================================

    const double scale =
        imageToScreenScale();

    if (scale <= 0.0)
    {
        event->accept();
        return;
    }

    const double step =
        MOVE_STEP_SCREEN / scale;

    const double halfWidth =
        m_cropPixelWidth / 2.0;

    const double halfHeight =
        m_cropPixelHeight / 2.0;

    const double leftEdge =
        halfWidth;

    const double rightEdge =
        m_image.width() - halfWidth;

    const double topEdge =
        halfHeight;

    const double bottomEdge =
        m_image.height() - halfHeight;

    if (event->nativeVirtualKey() == 0x58)
    {
        emit toggleOrientationRequested();

        event->accept();
        return;
    }

    switch (event->key())
    {
    case Qt::Key_Left:
        m_leftPressed = true;

        m_allowOverflowLeft =
            m_cropCenterX <=
            leftEdge + 0.0001;

        break;

    case Qt::Key_Right:
        m_rightPressed = true;

        m_allowOverflowRight =
            m_cropCenterX >=
            rightEdge - 0.0001;

        break;

    case Qt::Key_Up:
        m_upPressed = true;

        m_allowOverflowTop =
            m_cropCenterY <=
            topEdge + 0.0001;

        break;

    case Qt::Key_Down:
        m_downPressed = true;

        m_allowOverflowBottom =
            m_cropCenterY >=
            bottomEdge - 0.0001;

        break;

    default:
        QWidget::keyPressEvent(event);
        return;
    }

    moveCrop();

    if (!m_moveTimer->isActive())
        m_moveTimer->start();

    event->accept();
}

void CropCanvas::keyReleaseEvent(
    QKeyEvent* event
)
{
    // Игнорируем release, который является частью
    // системного автоповтора клавиши.
    if (event->isAutoRepeat())
    {
        event->accept();
        return;
    }

    switch (event->key())
    {
    case Qt::Key_Left:
        m_leftPressed = false;
        m_allowOverflowLeft = false;
        break;

    case Qt::Key_Right:
        m_rightPressed = false;
        m_allowOverflowRight = false;
        break;

    case Qt::Key_Up:
        m_upPressed = false;
        m_allowOverflowTop = false;
        break;

    case Qt::Key_Down:
        m_downPressed = false;
        m_allowOverflowBottom = false;
        break;

    default:
        QWidget::keyReleaseEvent(event);
        return;
    }

    if (
        !m_leftPressed &&
        !m_rightPressed &&
        !m_upPressed &&
        !m_downPressed
    )
    {
        m_moveTimer->stop();
    }

    event->accept();
}

void CropCanvas::moveCrop()
{
    if (m_image.isNull())
        return;

    const double scale =
        imageToScreenScale();

    if (scale <= 0.0)
        return;

    const double moveStep =
        MOVE_STEP_SCREEN / scale;

    const double maxOverflowX =
        (m_cropPixelWidth * (MAX_OVERFLOW_MM / 10.0))
        / m_cropWidthCm;

    const double maxOverflowY =
        (m_cropPixelHeight * (MAX_OVERFLOW_MM / 10.0))
        / m_cropHeightCm;

    const double halfWidth =
        m_cropPixelWidth / 2.0;

    const double halfHeight =
        m_cropPixelHeight / 2.0;

    const double leftEdge =
        halfWidth;

    const double rightEdge =
        m_image.width() - halfWidth;

    const double topEdge =
        halfHeight;

    const double bottomEdge =
        m_image.height() - halfHeight;


    // =========================================================
    // ВЛЕВО
    // =========================================================

    if (m_leftPressed)
    {
        const double minimum =
            m_allowOverflowLeft
                ? leftEdge - maxOverflowX
                : leftEdge;

        m_cropCenterX =
            std::max(
                minimum,
                m_cropCenterX - moveStep
            );
    }


    // =========================================================
    // ВПРАВО
    // =========================================================

    if (m_rightPressed)
    {
        const double maximum =
            m_allowOverflowRight
                ? rightEdge + maxOverflowX
                : rightEdge;

        m_cropCenterX =
            std::min(
                maximum,
                m_cropCenterX + moveStep
            );
    }


    // =========================================================
    // ВВЕРХ
    // =========================================================

    if (m_upPressed)
    {
        const double minimum =
            m_allowOverflowTop
                ? topEdge - maxOverflowY
                : topEdge;

        m_cropCenterY =
            std::max(
                minimum,
                m_cropCenterY - moveStep
            );
    }


    // =========================================================
    // ВНИЗ
    // =========================================================

    if (m_downPressed)
    {
        const double maximum =
            m_allowOverflowBottom
                ? bottomEdge + maxOverflowY
                : bottomEdge;

        m_cropCenterY =
            std::min(
                maximum,
                m_cropCenterY + moveStep
            );
    }

    update();
}

void CropCanvas::updateScaledImage()
{
    if (m_image.isNull())
    {
        m_scaledImage = QImage();
        m_lightPreviewSource = QImage();
        return;
    }

    const QSizeF availableSize = size();

    const double scale =
        std::min(
            availableSize.width() /
            m_image.width(),

            availableSize.height() /
            m_image.height()
        );

    const int width =
        qMax(
            1,
            static_cast<int>(
                m_image.width() * scale
            )
        );

    const int height =
        qMax(
            1,
            static_cast<int>(
                m_image.height() * scale
            )
        );

    // =========================================================
    // Свет
    // =========================================================

    if (m_showCropResult &&
        !m_lightSourceImage.isNull())
    {
        m_lightPreviewSource =
            m_lightSourceImage.scaled(
                width,
                height,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            );

        m_scaledImage =
            applyLightEffects(
                m_lightPreviewSource
            );

        return;
    }

    // =========================================================
    // Обычный режим
    // =========================================================

    m_lightPreviewSource =
        QImage();

    m_scaledImage =
        m_image.scaled(
            width,
            height,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
}

void CropCanvas::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    updateScaledImage();

    update();
}

double CropCanvas::imageToScreenScale() const
{
    if (m_image.isNull())
        return 1.0;

    const QRectF imageRect =
        imageRectOnScreen();

    return imageRect.width()
        / m_image.width();
}

double CropCanvas::maxOverflowScreenPixels(
    bool horizontal
) const
{
    if (m_image.isNull() ||
        m_cropPixelWidth <= 0.0 ||
        m_cropPixelHeight <= 0.0)
    {
        return 0.0;
    }

    const double outputPixels =
        MAX_OVERFLOW_MM * 300.0 / 25.4;

    const QRectF cropRect =
        cropRectOnScreen();

    const double scale =
        horizontal
            ? cropRect.width() / m_cropPixelWidth
            : cropRect.height() / m_cropPixelHeight;

    return outputPixels * scale;
}

double CropCanvas::screenToImagePixels(
    double screenPixels
) const
{
    const double scale =
        imageToScreenScale();

    if (scale <= 0.0)
        return 0.0;

    return screenPixels / scale;
}

void CropCanvas::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
    {
        event->ignore();
        return;
    }

    if (m_fitImageMode)
    {
        event->accept();
        return;
    }

    if (m_image.isNull())
    {
        emit emptyCanvasClicked();
        event->accept();
        return;
    }

    const QPointF position =
        event->position();

    const ResizeHandle handle =
        resizeHandleAt(position);

    if (handle != ResizeHandle::None)
    {
        m_mouseMode = MouseMode::Resize;
        m_resizeHandle = handle;

        m_lastMousePosition = position;

        m_resizeStartMousePosition =
            position;

        m_resizeStartWidth =
            m_cropPixelWidth;

        m_resizeStartHeight =
            m_cropPixelHeight;

        m_resizeStartCenterX =
            m_cropCenterX;

        m_resizeStartCenterY =
            m_cropCenterY;
        event->accept();
        return;
    }

    if (isInsideCropRect(position))
    {
        m_mouseMode = MouseMode::Move;
        m_resizeHandle = ResizeHandle::None;

        m_lastMousePosition = position;
        event->accept();
        return;
    }

    event->ignore();
}

void CropCanvas::setDisplayImage(
    const QImage& image,
    bool showCropResult
)
{
    if (showCropResult)
    {
        m_lightSourceImage = image;

        m_image = image;
    }
    else
    {
        m_lightSourceImage = QImage();
        m_lightPreviewSource = QImage();

        m_image = image;

        m_lightSettings = LightSettings{};
    }

    m_showCropResult =
        showCropResult;

    if (m_image.isNull())
    {
        m_scaledImage = QImage();
        return;
    }

    updateScaledImage();

    update();
}

double CropCanvas::cropPixelHeight() const
{
    return m_cropPixelHeight;
}

double CropCanvas::cropPixelWidth() const
{
    return m_cropPixelWidth;
}

CropCanvas::ResizeHandle
CropCanvas::resizeHandleAt(
    const QPointF& position
) const
{
    const QRectF rect =
        cropRectOnScreen();

    constexpr double HANDLE_SIZE = 14.0;

    const QRectF topLeft(
        rect.left() - HANDLE_SIZE,
        rect.top() - HANDLE_SIZE,
        HANDLE_SIZE * 2,
        HANDLE_SIZE * 2
    );

    const QRectF topRight(
        rect.right() - HANDLE_SIZE,
        rect.top() - HANDLE_SIZE,
        HANDLE_SIZE * 2,
        HANDLE_SIZE * 2
    );

    const QRectF bottomLeft(
        rect.left() - HANDLE_SIZE,
        rect.bottom() - HANDLE_SIZE,
        HANDLE_SIZE * 2,
        HANDLE_SIZE * 2
    );

    const QRectF bottomRight(
        rect.right() - HANDLE_SIZE,
        rect.bottom() - HANDLE_SIZE,
        HANDLE_SIZE * 2,
        HANDLE_SIZE * 2
    );

    if (topLeft.contains(position))
        return ResizeHandle::TopLeft;

    if (topRight.contains(position))
        return ResizeHandle::TopRight;

    if (bottomLeft.contains(position))
        return ResizeHandle::BottomLeft;

    if (bottomRight.contains(position))
        return ResizeHandle::BottomRight;

    constexpr double SIDE_TOLERANCE = 12.0;

    const bool nearLeft =
        qAbs(position.x() - rect.left())
        <= SIDE_TOLERANCE;

    const bool nearRight =
        qAbs(position.x() - rect.right())
        <= SIDE_TOLERANCE;

    const bool nearTop =
        qAbs(position.y() - rect.top())
        <= SIDE_TOLERANCE;

    const bool nearBottom =
        qAbs(position.y() - rect.bottom())
        <= SIDE_TOLERANCE;

    const bool insideVertical =
        position.y() >= rect.top() &&
        position.y() <= rect.bottom();

    const bool insideHorizontal =
        position.x() >= rect.left() &&
        position.x() <= rect.right();

    if (nearLeft && insideVertical)
        return ResizeHandle::Left;

    if (nearRight && insideVertical)
        return ResizeHandle::Right;

    if (nearTop && insideHorizontal)
        return ResizeHandle::Top;

    if (nearBottom && insideHorizontal)
        return ResizeHandle::Bottom;

    return ResizeHandle::None;
}

bool CropCanvas::isInsideCropRect(
    const QPointF& position
) const
{
    return cropRectOnScreen().contains(position);
}

void CropCanvas::mouseMoveEvent(
    QMouseEvent* event)
{
    if (m_fitImageMode)
    {
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }

    if (m_fitImageMode)
    {
        event->accept();
        return;
    }

    if (m_image.isNull())
        return;

    const QPointF position =
        event->position();

    // Right resize
    if (m_mouseMode == MouseMode::Resize &&
        m_resizeHandle == ResizeHandle::Right &&
        (event->buttons() & Qt::LeftButton))
    {
        const QRectF imageRect =
            imageRectOnScreen();

        const double scale =
            imageToScreenScale();

        if (scale <= 0.0)
        {
            event->accept();
            return;
        }

        // Левая граница запоминается с момента
        // начала изменения размера и больше не двигается.
        const double left =
            m_resizeStartCenterX -
            m_resizeStartWidth / 2.0;

        // Положение курсора переводим
        // из экранных координат в координаты изображения.
        const double mouseX =
        (
            event->position().x()
            - imageRect.left()
        ) / scale;

        // Курсор задаёт новую правую границу.
        const double right =
            mouseX;

        // Новая ширина определяется только
        // расстоянием от неподвижной левой
        // границы до новой правой.
        double newWidth =
            right - left;

        newWidth =
            limitResizeWidth(
                ResizeHandle::Right,
                newWidth
            );

        const double aspectRatio =
            m_resizeStartWidth /
            m_resizeStartHeight;

        const double newHeight =
            newWidth / aspectRatio;

        // Левая граница остаётся прежней.
        // Правая определяется новой шириной.
        const double newRight =
            left + newWidth;

        m_cropPixelWidth =
            newWidth;

        m_cropPixelHeight =
            newHeight;

        // Центр находится ровно между
        // неподвижной левой и новой правой границей.
        m_cropCenterX =
            (left + newRight) / 2.0;

        // По вертикали центр остаётся неподвижным.
        m_cropCenterY =
            m_resizeStartCenterY;

        update();

        event->accept();
        return;
    }
    // Left-resize
    if (m_mouseMode == MouseMode::Resize &&
        m_resizeHandle == ResizeHandle::Left &&
        (event->buttons() & Qt::LeftButton))
    {
        const QRectF imageRect =
            imageRectOnScreen();

        const double scale =
            imageToScreenScale();

        if (scale <= 0.0)
        {
            event->accept();
            return;
        }

        // Правая граница остаётся неподвижной.
        const double right =
            m_resizeStartCenterX +
            m_resizeStartWidth / 2.0;

        // Переводим положение мыши
        // из экранных координат в координаты изображения.
        const double mouseX =
        (
            event->position().x()
            - imageRect.left()
        ) / scale;

        // Курсор задаёт новую левую границу.
        const double left =
            mouseX;

        // Новая ширина определяется расстоянием
        // от новой левой до неподвижной правой.
        double newWidth =
            right - left;

        newWidth =
            limitResizeWidth(
                ResizeHandle::Left,
                newWidth
            );

        const double aspectRatio =
            m_resizeStartWidth /
            m_resizeStartHeight;

        const double newHeight =
            newWidth / aspectRatio;

        // Левая граница — новая.
        // Правая — старая.
        const double newLeft =
            right - newWidth;

        m_cropPixelWidth =
            newWidth;

        m_cropPixelHeight =
            newHeight;

        // Центр находится между новой левой
        // и неподвижной правой границей.
        m_cropCenterX =
            (newLeft + right) / 2.0;

        // Вертикальный центр остаётся неподвижным.
        m_cropCenterY =
            m_resizeStartCenterY;

        update();

        event->accept();
        return;
    }

    // Top-resize
    if (m_mouseMode == MouseMode::Resize &&
        m_resizeHandle == ResizeHandle::Top &&
        (event->buttons() & Qt::LeftButton))
    {
        const QRectF imageRect =
            imageRectOnScreen();

        const double scale =
            imageToScreenScale();

        if (scale <= 0.0)
        {
            event->accept();
            return;
        }

        // Нижняя граница остаётся неподвижной.
        const double bottom =
            m_resizeStartCenterY +
            m_resizeStartHeight / 2.0;

        // Переводим положение мыши
        // из экранных координат в координаты изображения.
        const double mouseY =
        (
            event->position().y()
            - imageRect.top()
        ) / scale;

        // Курсор задаёт новую верхнюю границу.
        const double top =
            mouseY;

        // Новая высота определяется расстоянием
        // от новой верхней до неподвижной нижней.
        double newHeight =
            bottom - top;

        const double aspectRatio =
            m_resizeStartWidth /
            m_resizeStartHeight;

        double newWidth =
            newHeight * aspectRatio;

        newWidth =
            limitResizeWidth(
                ResizeHandle::Top,
                newWidth
            );

        newHeight =
            newWidth / aspectRatio;

        // Верхняя граница — новая.
        // Нижняя — старая.
        const double newTop =
            bottom - newHeight;

        m_cropPixelWidth =
            newWidth;

        m_cropPixelHeight =
            newHeight;

        // Вертикальный центр находится
        // между новой верхней и неподвижной
        // нижней границей.
        m_cropCenterY =
            (newTop + bottom) / 2.0;

        // Горизонтальный центр остаётся неподвижным.
        m_cropCenterX =
            m_resizeStartCenterX;

        update();

        event->accept();
        return;
    }

    // Bottom-resize
    if (m_mouseMode == MouseMode::Resize &&
        m_resizeHandle == ResizeHandle::Bottom &&
        (event->buttons() & Qt::LeftButton))
    {
        const QRectF imageRect =
            imageRectOnScreen();

        const double scale =
            imageToScreenScale();

        if (scale <= 0.0)
        {
            event->accept();
            return;
        }

        // Верхняя граница остаётся неподвижной.
        const double top =
            m_resizeStartCenterY -
            m_resizeStartHeight / 2.0;

        // Переводим положение мыши
        // из экранных координат в координаты изображения.
        const double mouseY =
        (
            event->position().y()
            - imageRect.top()
        ) / scale;

        // Курсор задаёт новую нижнюю границу.
        const double bottom =
            mouseY;

        // Новая высота определяется расстоянием
        // от неподвижной верхней до новой нижней границы.
        double newHeight =
            bottom - top;

        const double aspectRatio =
            m_resizeStartWidth /
            m_resizeStartHeight;

        double newWidth =
            newHeight * aspectRatio;

        newWidth =
            limitResizeWidth(
                ResizeHandle::Bottom,
                newWidth
            );

        newHeight =
            newWidth / aspectRatio;

        // Верхняя граница остаётся прежней.
        // Нижняя — новая.
        const double newBottom =
            top + newHeight;

        m_cropPixelWidth =
            newWidth;

        m_cropPixelHeight =
            newHeight;

        // Центр находится между неподвижной
        // верхней и новой нижней границей.
        m_cropCenterY =
            (top + newBottom) / 2.0;

        // Горизонтальный центр остаётся неподвижным.
        m_cropCenterX =
            m_resizeStartCenterX;

        update();

        event->accept();
        return;
    }
    // TopRight-resize
    if (m_mouseMode == MouseMode::Resize &&
        m_resizeHandle == ResizeHandle::TopRight &&
        (event->buttons() & Qt::LeftButton))
    {
        const double scale =
            imageToScreenScale();

        if (scale <= 0.0)
        {
            event->accept();
            return;
        }

        // Неподвижные границы:
        // левая и нижняя.
        const double left =
            m_resizeStartCenterX -
            m_resizeStartWidth / 2.0;

        const double bottom =
            m_resizeStartCenterY +
            m_resizeStartHeight / 2.0;

        const double aspectRatio =
            m_resizeStartWidth /
            m_resizeStartHeight;

        // Разрешённое направление движения
        // верхнего правого угла.
        //
        // При увеличении:
        // X растёт
        // Y уменьшается
        const QPointF resizeDirection(
            aspectRatio,
            -1.0
        );

        const QPointF mouseDelta =
            event->position()
            - m_resizeStartMousePosition;

        // Проекция движения мыши
        // на направление изменения размера.
        const double directionLengthSquared =
            QPointF::dotProduct(
                resizeDirection,
                resizeDirection
            );

        const double projection =
            QPointF::dotProduct(
                mouseDelta,
                resizeDirection
            ) / directionLengthSquared;

        // Переводим проекцию в изменение
        // ширины в экранных координатах.
        const double deltaWidthScreen =
            projection *
            aspectRatio;

        const double deltaWidthImage =
            deltaWidthScreen / scale;

        double newWidth =
            m_resizeStartWidth +
            deltaWidthImage;

        newWidth =
            limitResizeWidth(
                ResizeHandle::TopRight,
                newWidth
            );

        const double newHeight =
            newWidth / aspectRatio;

        const double right =
            left + newWidth;

        const double top =
            bottom - newHeight;

        m_cropPixelWidth =
            newWidth;

        m_cropPixelHeight =
            newHeight;

        m_cropCenterX =
            (left + right) / 2.0;

        m_cropCenterY =
            (top + bottom) / 2.0;

        update();

        event->accept();
        return;
    }

    // TopLeft-resize
    if (m_mouseMode == MouseMode::Resize &&
        m_resizeHandle == ResizeHandle::TopLeft &&
        (event->buttons() & Qt::LeftButton))
    {
        const double scale =
            imageToScreenScale();

        if (scale <= 0.0)
        {
            event->accept();
            return;
        }

        // Неподвижные границы:
        // правая и нижняя.
        const double right =
            m_resizeStartCenterX +
            m_resizeStartWidth / 2.0;

        const double bottom =
            m_resizeStartCenterY +
            m_resizeStartHeight / 2.0;

        const double aspectRatio =
            m_resizeStartWidth /
            m_resizeStartHeight;

        // TopLeft:
        // X уменьшается
        // Y уменьшается
        const QPointF resizeDirection(
            -aspectRatio,
            -1.0
        );

        const QPointF mouseDelta =
            event->position()
            - m_resizeStartMousePosition;

        const double directionLengthSquared =
            QPointF::dotProduct(
                resizeDirection,
                resizeDirection
            );

        const double projection =
            QPointF::dotProduct(
                mouseDelta,
                resizeDirection
            ) / directionLengthSquared;

        const double deltaWidthScreen =
            projection * aspectRatio;

        const double deltaWidthImage =
            deltaWidthScreen / scale;

        double newWidth =
            m_resizeStartWidth +
            deltaWidthImage;

        newWidth =
            limitResizeWidth(
                ResizeHandle::TopLeft,
                newWidth
            );

        const double newHeight =
            newWidth / aspectRatio;

        const double left =
            right - newWidth;

        const double top =
            bottom - newHeight;

        m_cropPixelWidth =
            newWidth;

        m_cropPixelHeight =
            newHeight;

        m_cropCenterX =
            (left + right) / 2.0;

        m_cropCenterY =
            (top + bottom) / 2.0;

        update();

        event->accept();
        return;
    }

    // BottomRight-resize
    if (m_mouseMode == MouseMode::Resize &&
        m_resizeHandle == ResizeHandle::BottomRight &&
        (event->buttons() & Qt::LeftButton))
    {
        const double scale =
            imageToScreenScale();

        if (scale <= 0.0)
        {
            event->accept();
            return;
        }

        // Неподвижные границы:
        // левая и верхняя.
        const double left =
            m_resizeStartCenterX -
            m_resizeStartWidth / 2.0;

        const double top =
            m_resizeStartCenterY -
            m_resizeStartHeight / 2.0;

        const double aspectRatio =
            m_resizeStartWidth /
            m_resizeStartHeight;

        // BottomRight:
        // X увеличивается
        // Y увеличивается
        const QPointF resizeDirection(
            aspectRatio,
            1.0
        );

        const QPointF mouseDelta =
            event->position()
            - m_resizeStartMousePosition;

        const double directionLengthSquared =
            QPointF::dotProduct(
                resizeDirection,
                resizeDirection
            );

        const double projection =
            QPointF::dotProduct(
                mouseDelta,
                resizeDirection
            ) / directionLengthSquared;

        const double deltaWidthScreen =
            projection * aspectRatio;

        const double deltaWidthImage =
            deltaWidthScreen / scale;

        double newWidth =
            m_resizeStartWidth +
            deltaWidthImage;

        newWidth =
            limitResizeWidth(
                ResizeHandle::BottomRight,
                newWidth
            );

        const double newHeight =
            newWidth / aspectRatio;

        const double right =
            left + newWidth;

        const double bottom =
            top + newHeight;

        m_cropPixelWidth =
            newWidth;

        m_cropPixelHeight =
            newHeight;

        m_cropCenterX =
            (left + right) / 2.0;

        m_cropCenterY =
            (top + bottom) / 2.0;

        update();

        event->accept();
        return;
    }

    // BottomLeft-resize
    if (m_mouseMode == MouseMode::Resize &&
        m_resizeHandle == ResizeHandle::BottomLeft &&
        (event->buttons() & Qt::LeftButton))
    {
        const double scale =
            imageToScreenScale();

        if (scale <= 0.0)
        {
            event->accept();
            return;
        }

        // Неподвижные границы:
        // правая и верхняя.
        const double right =
            m_resizeStartCenterX +
            m_resizeStartWidth / 2.0;

        const double top =
            m_resizeStartCenterY -
            m_resizeStartHeight / 2.0;

        const double aspectRatio =
            m_resizeStartWidth /
            m_resizeStartHeight;

        // BottomLeft:
        // X уменьшается
        // Y увеличивается
        const QPointF resizeDirection(
            -aspectRatio,
            1.0
        );

        const QPointF mouseDelta =
            event->position()
            - m_resizeStartMousePosition;

        const double directionLengthSquared =
            QPointF::dotProduct(
                resizeDirection,
                resizeDirection
            );

        const double projection =
            QPointF::dotProduct(
                mouseDelta,
                resizeDirection
            ) / directionLengthSquared;

        const double deltaWidthScreen =
            projection * aspectRatio;

        const double deltaWidthImage =
            deltaWidthScreen / scale;

        double newWidth =
            m_resizeStartWidth +
            deltaWidthImage;

        newWidth =
            limitResizeWidth(
                ResizeHandle::BottomLeft,
                newWidth
            );

        const double newHeight =
            newWidth / aspectRatio;

        const double left =
            right - newWidth;

        const double bottom =
            top + newHeight;

        m_cropPixelWidth =
            newWidth;

        m_cropPixelHeight =
            newHeight;

        m_cropCenterX =
            (left + right) / 2.0;

        m_cropCenterY =
            (top + bottom) / 2.0;

        update();

        event->accept();
        return;
    }

    if (m_mouseMode == MouseMode::Move &&
        (event->buttons() & Qt::LeftButton))
    {
        const QPointF delta =
            position - m_lastMousePosition;

        const double scale =
            imageToScreenScale();

        if (scale > 0.0)
        {
            const double deltaX =
                delta.x() / scale;

            const double deltaY =
                delta.y() / scale;

            m_cropCenterX += deltaX;
            m_cropCenterY += deltaY;

            const double halfWidth =
                m_cropPixelWidth / 2.0;

            const double halfHeight =
                m_cropPixelHeight / 2.0;

            m_cropCenterX =
                std::clamp(
                    m_cropCenterX,
                    halfWidth,
                    m_image.width() - halfWidth
                );

            m_cropCenterY =
                std::clamp(
                    m_cropCenterY,
                    halfHeight,
                    m_image.height() - halfHeight
                );
        }

        m_lastMousePosition = position;

        update();

        event->accept();
        return;
    }

    const ResizeHandle handle =
        resizeHandleAt(position);

    switch (handle)
    {
    case ResizeHandle::TopLeft:
    case ResizeHandle::BottomRight:
        setCursor(Qt::SizeFDiagCursor);
        break;

    case ResizeHandle::TopRight:
    case ResizeHandle::BottomLeft:
        setCursor(Qt::SizeBDiagCursor);
        break;

    case ResizeHandle::Left:
    case ResizeHandle::Right:
        setCursor(Qt::SizeHorCursor);
        break;

    case ResizeHandle::Top:
    case ResizeHandle::Bottom:
        setCursor(Qt::SizeVerCursor);
        break;

    case ResizeHandle::None:

        if (isInsideCropRect(position))
        {
            setCursor(
                Qt::SizeAllCursor
            );
        }
        else
        {
            setCursor(
                Qt::ArrowCursor
            );
        }

        break;
    }

    event->accept();
}

QRectF CropCanvas::constrainResizeRect(
    const QRectF& rect
) const
{
    if (m_image.isNull())
        return rect;

    QRectF result = rect;

    const QRectF imageRect(
        0.0,
        0.0,
        m_image.width(),
        m_image.height()
    );

    if (result.left() < imageRect.left())
    {
        result.moveLeft(
            imageRect.left()
        );
    }

    if (result.right() > imageRect.right())
    {
        result.moveRight(
            imageRect.right()
        );
    }

    if (result.top() < imageRect.top())
    {
        result.moveTop(
            imageRect.top()
        );
    }

    if (result.bottom() > imageRect.bottom())
    {
        result.moveBottom(
            imageRect.bottom()
        );
    }

    return result;
}

double CropCanvas::limitResizeWidth(
    ResizeHandle handle,
    double desiredWidth
) const
{
    if (m_image.isNull())
        return desiredWidth;

    if (m_resizeStartHeight <= 0.0)
        return desiredWidth;

    const double aspectRatio =
        m_resizeStartWidth /
        m_resizeStartHeight;

    if (aspectRatio <= 0.0)
        return desiredWidth;

    const double imageWidth =
        m_image.width();

    const double imageHeight =
        m_image.height();

    const double left =
        m_resizeStartCenterX -
        m_resizeStartWidth / 2.0;

    const double right =
        m_resizeStartCenterX +
        m_resizeStartWidth / 2.0;

    const double top =
        m_resizeStartCenterY -
        m_resizeStartHeight / 2.0;

    const double bottom =
        m_resizeStartCenterY +
        m_resizeStartHeight / 2.0;

    double maxWidth =
        std::numeric_limits<double>::max();

    switch (handle)
    {
    case ResizeHandle::Right:
        {
            // Левая граница неподвижна.
            const double maxWidthHorizontal =
                imageWidth - left;

            // Центр по Y неподвижен.
            const double maxHeightVertical =
                2.0 * std::min(
                    m_resizeStartCenterY,
                    imageHeight -
                    m_resizeStartCenterY
                );

            const double maxWidthVertical =
                maxHeightVertical *
                aspectRatio;

            maxWidth =
                std::min(
                    maxWidthHorizontal,
                    maxWidthVertical
                );

            break;
        }

    case ResizeHandle::Left:
        {
            // Правая граница неподвижна.
            const double maxWidthHorizontal =
                right;

            // Центр по Y неподвижен.
            const double maxHeightVertical =
                2.0 * std::min(
                    m_resizeStartCenterY,
                    imageHeight -
                    m_resizeStartCenterY
                );

            const double maxWidthVertical =
                maxHeightVertical *
                aspectRatio;

            maxWidth =
                std::min(
                    maxWidthHorizontal,
                    maxWidthVertical
                );

            break;
        }

    case ResizeHandle::Top:
        {
            // Нижняя граница неподвижна.
            const double maxHeightVertical =
                bottom;

            // Центр по X неподвижен.
            const double maxWidthHorizontal =
                2.0 * std::min(
                    m_resizeStartCenterX,
                    imageWidth -
                    m_resizeStartCenterX
                );

            const double maxHeightHorizontal =
                maxWidthHorizontal /
                aspectRatio;

            const double maxHeight =
                std::min(
                    maxHeightVertical,
                    maxHeightHorizontal
                );

            maxWidth =
                maxHeight *
                aspectRatio;

            break;
        }

    case ResizeHandle::Bottom:
        {
            // Верхняя граница неподвижна.
            const double maxHeightVertical =
                imageHeight - top;

            // Центр по X неподвижен.
            const double maxWidthHorizontal =
                2.0 * std::min(
                    m_resizeStartCenterX,
                    imageWidth -
                    m_resizeStartCenterX
                );

            const double maxHeightHorizontal =
                maxWidthHorizontal /
                aspectRatio;

            const double maxHeight =
                std::min(
                    maxHeightVertical,
                    maxHeightHorizontal
                );

            maxWidth =
                maxHeight *
                aspectRatio;

            break;
        }

    case ResizeHandle::TopLeft:
        {
            // Неподвижные границы:
            // правая и нижняя.
            const double maxWidthHorizontal =
                right;

            const double maxHeightVertical =
                bottom;

            const double maxWidthVertical =
                maxHeightVertical *
                aspectRatio;

            maxWidth =
                std::min(
                    maxWidthHorizontal,
                    maxWidthVertical
                );

            break;
        }

    case ResizeHandle::TopRight:
        {
            // Неподвижные границы:
            // левая и нижняя.
            const double maxWidthHorizontal =
                imageWidth - left;

            const double maxHeightVertical =
                bottom;

            const double maxWidthVertical =
                maxHeightVertical *
                aspectRatio;

            maxWidth =
                std::min(
                    maxWidthHorizontal,
                    maxWidthVertical
                );

            break;
        }

    case ResizeHandle::BottomLeft:
        {
            // Неподвижные границы:
            // правая и верхняя.
            const double maxWidthHorizontal =
                right;

            const double maxHeightVertical =
                imageHeight - top;

            const double maxWidthVertical =
                maxHeightVertical *
                aspectRatio;

            maxWidth =
                std::min(
                    maxWidthHorizontal,
                    maxWidthVertical
                );

            break;
        }

    case ResizeHandle::BottomRight:
        {
            // Неподвижные границы:
            // левая и верхняя.
            const double maxWidthHorizontal =
                imageWidth - left;

            const double maxHeightVertical =
                imageHeight - top;

            const double maxWidthVertical =
                maxHeightVertical *
                aspectRatio;

            maxWidth =
                std::min(
                    maxWidthHorizontal,
                    maxWidthVertical
                );

            break;
        }

    case ResizeHandle::None:
        return desiredWidth;
    }

    return std::clamp(
        desiredWidth,
        10.0,
        maxWidth
    );
}

void CropCanvas::setCropPixelSize(
    double width,
    double height)
{
    if (width <= 0.0 || height <= 0.0)
        return;

    m_cropPixelWidth = width;
    m_cropPixelHeight = height;

    update();
}

void CropCanvas::toggleFitImageMode()
{
    if (!m_fitImageMode)
    {
        // Первое Shift+Enter:
        // входим в режим подгонки
        // без дополнительных полей.

        m_fitImageMode = true;
        m_fitImageWithMargins = false;
    }
    else
    {
        // Следующие Shift+Enter:
        // переключаем:
        //
        // 0 мм <-> 4 мм

        m_fitImageWithMargins =
            !m_fitImageWithMargins;
    }

    // В режиме подгонки:
    // рамку нельзя двигать;
    // рамку нельзя изменять.

    m_mouseMode =
        MouseMode::None;

    m_resizeHandle =
        ResizeHandle::None;

    setCursor(
        Qt::ArrowCursor
    );

    update();
}

void CropCanvas::exitFitImageMode()
{
    m_fitImageMode = false;

    m_fitImageWithMargins = false;

    m_mouseMode =
        MouseMode::None;

    m_resizeHandle =
        ResizeHandle::None;

    setCursor(
        Qt::ArrowCursor
    );

    update();
}

bool CropCanvas::isFitImageMode() const
{
    return m_fitImageMode;
}

QRectF CropCanvas::displayImageRectOnScreen() const
{
    const QRectF imageRect =
        imageRectOnScreen();

    if (!m_fitImageMode ||
        m_image.isNull())
    {
        return imageRect;
    }

    const QRectF cropRect =
        cropRectOnScreen();

    double marginX = 0.0;
    double marginY = 0.0;

    if (m_fitImageWithMargins)
    {
        // 4 мм = 0.4 см
        marginX =
            cropRect.width()
            * (0.4 / m_cropWidthCm);

        marginY =
            cropRect.height()
            * (0.4 / m_cropHeightCm);
    }

    const QRectF availableRect =
        cropRect.adjusted(
            marginX,
            marginY,
            -marginX,
            -marginY
        );

    if (availableRect.width() <= 0.0 ||
        availableRect.height() <= 0.0)
    {
        return QRectF();
    }

    const double imageAspect =
        static_cast<double>(m_image.width())
        / static_cast<double>(m_image.height());

    const double availableAspect =
        availableRect.width()
        / availableRect.height();

    double width;
    double height;

    if (imageAspect > availableAspect)
    {
        // Изображение шире области.
        width = availableRect.width();
        height = width / imageAspect;
    }
    else
    {
        // Изображение выше области.
        height = availableRect.height();
        width = height * imageAspect;
    }

    const double x =
        availableRect.left()
        + (availableRect.width() - width) / 2.0;

    const double y =
        availableRect.top()
        + (availableRect.height() - height) / 2.0;

    return QRectF(
        x,
        y,
        width,
        height
    );
}

bool CropCanvas::fitImageWithMargins() const
{
    return m_fitImageWithMargins;
}

void CropCanvas::restoreFitImageMode(
    bool withMargins
)
{
    m_fitImageMode = true;
    m_fitImageWithMargins = withMargins;

    m_mouseMode =
        MouseMode::None;

    m_resizeHandle =
        ResizeHandle::None;

    setCursor(Qt::ArrowCursor);

    update();
}

QImage CropCanvas::createFitCropImage() const
{
    if (m_image.isNull())
        return QImage();

    const int outputWidth =
        qRound(m_cropPixelWidth);

    const int outputHeight =
        qRound(m_cropPixelHeight);

    if (outputWidth <= 0 ||
        outputHeight <= 0)
    {
        return QImage();
    }

    QImage result(
        outputWidth,
        outputHeight,
        QImage::Format_ARGB32
    );

    result.fill(Qt::white);

    double marginX = 0.0;
    double marginY = 0.0;

    if (m_fitImageWithMargins)
    {
        marginX =
            outputWidth *
            (0.4 / m_cropWidthCm);

        marginY =
            outputHeight *
            (0.4 / m_cropHeightCm);
    }

    const QRectF availableRect(
        marginX,
        marginY,
        outputWidth - 2.0 * marginX,
        outputHeight - 2.0 * marginY
    );

    if (availableRect.width() <= 0.0 ||
        availableRect.height() <= 0.0)
    {
        return result;
    }

    const double imageAspect =
        static_cast<double>(m_image.width())
        /
        static_cast<double>(m_image.height());

    const double availableAspect =
        availableRect.width()
        /
        availableRect.height();

    double drawWidth;
    double drawHeight;

    if (imageAspect > availableAspect)
    {
        drawWidth =
            availableRect.width();

        drawHeight =
            drawWidth / imageAspect;
    }
    else
    {
        drawHeight =
            availableRect.height();

        drawWidth =
            drawHeight * imageAspect;
    }

    const double x =
        availableRect.left()
        +
        (
            availableRect.width()
            - drawWidth
        ) / 2.0;

    const double y =
        availableRect.top()
        +
        (
            availableRect.height()
            - drawHeight
        ) / 2.0;

    QPainter painter(&result);

    painter.setRenderHint(
        QPainter::SmoothPixmapTransform,
        true
    );

    painter.drawImage(
        QRectF(
            x,
            y,
            drawWidth,
            drawHeight
        ),
        m_image
    );

    painter.end();

    return result;
}

QImage CropCanvas::applyLevels(
    const QImage& image,
    int status
) const
{
    if (image.isNull())
        return QImage();

    const auto lut =
        createLevelsLut(status);

    QImage result =
        image.convertToFormat(
            QImage::Format_ARGB32
        );

    for (int y = 0;
         y < result.height();
         ++y)
    {
        QRgb* line =
            reinterpret_cast<QRgb*>(
                result.scanLine(y)
            );

        for (int x = 0;
             x < result.width();
             ++x)
        {
            const QRgb pixel =
                line[x];

            line[x] =
                qRgba(
                    lut[qRed(pixel)],
                    lut[qGreen(pixel)],
                    lut[qBlue(pixel)],
                    qAlpha(pixel)
                );
        }
    }

    return result;
}

std::array<uchar, 256>
CropCanvas::createCurvesLut(int status) const
{
    std::array<uchar, 256> lut{};

    status =
        std::clamp(
            status,
            MIN_LIGHT_STATUS,
            MAX_LIGHT_STATUS
        );

    constexpr double inputPoint = 113.0;

    double outputPoint = 113.0;

    switch (status)
    {
    case -3:
        outputPoint = 70.0;
        break;

    case -2:
        outputPoint = 84.0;
        break;

    case -1:
        outputPoint = 99.0;
        break;

    case 0:
        outputPoint = 113.0;
        break;

    case 1:
        outputPoint = 127.0;
        break;

    case 2:
        outputPoint = 142.0;
        break;

    case 3:
        outputPoint = 156.0;
        break;
    }

    // ---------------------------------------------------------
    // Нулевая кривая
    // ---------------------------------------------------------

    if (status == 0)
    {
        for (int i = 0; i < 256; ++i)
            lut[i] = static_cast<uchar>(i);

        return lut;
    }

    /*
     * Используем кубическую Hermite-интерполяцию.
     *
     * Три основные точки:
     *
     *      (0, 0)
     *      (113, outputPoint)
     *      (255, 255)
     *
     * В отличие от предыдущего варианта,
     * здесь форма кривой определяется непосредственно
     * этими точками и их наклонами.
     */

    const double leftSlope =
        outputPoint / inputPoint;

    const double rightSlope =
        (255.0 - outputPoint)
        /
        (255.0 - inputPoint);

    // Наклон в центральной точке.
    //
    // Среднее между наклоном левой
    // и правой частей.
    const double middleSlope =
        (leftSlope + rightSlope) / 2.0;

    // ---------------------------------------------------------
    // Левая часть
    // ---------------------------------------------------------

    for (int x = 0;
         x <= static_cast<int>(inputPoint);
         ++x)
    {
        const double t =
            static_cast<double>(x)
            /
            inputPoint;

        const double h =
            inputPoint;

        const double p0 = 0.0;
        const double p1 = outputPoint;

        const double m0 = leftSlope;
        const double m1 = middleSlope;

        const double t2 = t * t;
        const double t3 = t2 * t;

        const double h00 =
            2.0 * t3
            - 3.0 * t2
            + 1.0;

        const double h10 =
            t3
            - 2.0 * t2
            + t;

        const double h01 =
            -2.0 * t3
            + 3.0 * t2;

        const double h11 =
            t3
            - t2;

        const double result =
            h00 * p0
            +
            h10 * h * m0
            +
            h01 * p1
            +
            h11 * h * m1;

        lut[x] =
            static_cast<uchar>(
                std::clamp(
                    qRound(result),
                    0,
                    255
                )
            );
    }

    // ---------------------------------------------------------
    // Правая часть
    // ---------------------------------------------------------

    for (int x =
             static_cast<int>(inputPoint);
         x < 256;
         ++x)
    {
        const double t =
            static_cast<double>(
                x - inputPoint
            )
            /
            (255.0 - inputPoint);

        const double h =
            255.0 - inputPoint;

        const double p0 =
            outputPoint;

        const double p1 =
            255.0;

        const double m0 =
            middleSlope;

        const double m1 =
            rightSlope;

        const double t2 = t * t;
        const double t3 = t2 * t;

        const double h00 =
            2.0 * t3
            - 3.0 * t2
            + 1.0;

        const double h10 =
            t3
            - 2.0 * t2
            + t;

        const double h01 =
            -2.0 * t3
            + 3.0 * t2;

        const double h11 =
            t3
            - t2;

        const double result =
            h00 * p0
            +
            h10 * h * m0
            +
            h01 * p1
            +
            h11 * h * m1;

        lut[x] =
            static_cast<uchar>(
                std::clamp(
                    qRound(result),
                    0,
                    255
                )
            );
    }

    return lut;
}

std::array<uchar, 256>
CropCanvas::createLevelsLut(int status) const
{
    std::array<uchar, 256> lut{};

    status =
        std::clamp(
            status,
            MIN_LIGHT_STATUS,
            MAX_LIGHT_STATUS
        );

    double gamma = 1.0;
    int whitePoint = 255;

    switch (status)
    {
    case -3:
        gamma = 0.85;
        break;

    case -2:
        gamma = 0.90;
        break;

    case -1:
        gamma = 0.95;
        break;

    case 0:
        gamma = 1.00;
        break;

    case 1:
        whitePoint = 245;
        break;

    case 2:
        whitePoint = 235;
        break;

    case 3:
        whitePoint = 225;
        break;
    }

    for (int value = 0; value < 256; ++value)
    {
        double normalized =
            static_cast<double>(value)
            /
            static_cast<double>(whitePoint);

        normalized =
            std::clamp(
                normalized,
                0.0,
                1.0
            );

        const double corrected =
            std::pow(
                normalized,
                gamma
            );

        lut[value] =
            static_cast<uchar>(
                std::clamp(
                    qRound(
                        corrected * 255.0
                    ),
                    0,
                    255
                )
            );
    }

    return lut;
}

QImage CropCanvas::applyLightEffects(
    const QImage& image
) const
{
    QImage result = image;

    if (m_lightSettings.levels != 0)
    {
        result =
            applyLevels(
                result,
                m_lightSettings.levels
            );
    }

    if (m_lightSettings.curves != 0)
    {
        result =
            applyCurves(
                result,
                m_lightSettings.curves
            );
    }

    return result;
}

void CropCanvas::updateLightPreview()
{
    if (m_lightPreviewSource.isNull())
        return;

    m_scaledImage =
        applyLightEffects(
            m_lightPreviewSource
        );

    update();
}

void CropCanvas::setLevelsStatus(int status)
{
    m_lightSettings.levels =
        std::clamp(
            status,
            MIN_LIGHT_STATUS,
            MAX_LIGHT_STATUS
        );

    updateLightPreview();
}

void CropCanvas::setCurvesStatus(int status)
{
    m_lightSettings.curves =
        std::clamp(
            status,
            MIN_LIGHT_STATUS,
            MAX_LIGHT_STATUS
        );

    updateLightPreview();
}

QImage CropCanvas::createFinalLightImage() const
{
    if (m_lightSourceImage.isNull())
        return QImage();

    return applyLightEffects(
        m_lightSourceImage
    );
}

QImage CropCanvas::applyCurves(
    const QImage& image,
    int status
) const
{
    if (image.isNull())
        return QImage();

    const auto lut =
        createCurvesLut(status);

    if (status == 0)
        return image;

    QImage result =
        image.convertToFormat(
            QImage::Format_ARGB32
        );

    for (int y = 0;
         y < result.height();
         ++y)
    {
        QRgb* line =
            reinterpret_cast<QRgb*>(
                result.scanLine(y)
            );

        for (int x = 0;
             x < result.width();
             ++x)
        {
            const QRgb pixel =
                line[x];

            line[x] =
                qRgba(
                    lut[qRed(pixel)],
                    lut[qGreen(pixel)],
                    lut[qBlue(pixel)],
                    qAlpha(pixel)
                );
        }
    }

    return result;
}
