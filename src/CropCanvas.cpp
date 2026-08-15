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

    painter.fillRect(
        rect(),
        QColor(128, 128, 128)
    );

    if (m_image.isNull())
    {
        painter.setPen(Qt::white);

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

    painter.drawImage(
        imageRect,
        m_scaledImage
    );


    const QRectF cropRect =
        cropRectOnScreen();

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

    if (event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter)
    {
        emit forwardRequested();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Backspace)
    {
        emit backRequested();
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
        return;
    }

    const QSizeF availableSize = size();

    const double scale =
        std::min(
            availableSize.width() / m_image.width(),
            availableSize.height() / m_image.height()
        );

    const int width =
        qMax(1, static_cast<int>(
                 m_image.width() * scale
             ));

    const int height =
        qMax(1, static_cast<int>(
                 m_image.height() * scale
             ));

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

void CropCanvas::setDisplayImage(const QImage& image)
{
    m_image = image;

    if (m_image.isNull())
    {
        m_scaledImage = QImage();
        update();
        return;
    }

    updateScaledImage();

    update();
}

void CropCanvas::setDisplayImage(
    const QImage& image,
    bool showCropResult)
{
    m_image = image;
    m_showCropResult = showCropResult;

    if (m_image.isNull())
    {
        m_scaledImage = QImage();
        update();
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
