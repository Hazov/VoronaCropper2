#pragma once

#include <QWidget>
#include <QImage>
#include <QTimer>
#include <QMouseEvent>
#include <QRectF>

struct LightSettings
{
    int levels = 0;
    int curves = 0;
};

class CropCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit CropCanvas(QWidget* parent = nullptr);

    void setImage(const QImage& image);

    void clearImage();

    void setCropRatio(double width, double height);

    void resetCrop();

    double cropCenterX() const;
    double cropCenterY() const;

    QRectF cropRectInImage() const;

    void setCropCenter(double x, double y);

    void setDisplayImage(const QImage& image);

    void setDisplayImage(
        const QImage& image,
        bool showCropResult
    );

    double cropPixelHeight() const;
    double cropPixelWidth() const;

    void setCropPixelSize(
        double width,
        double height
    );
    void toggleFitImageMode();
    void exitFitImageMode();
    bool isFitImageMode() const;
    QRectF displayImageRectOnScreen() const;

    bool fitImageWithMargins() const;

    void restoreFitImageMode(
        bool withMargins
    );

    QImage createFitCropImage() const;

    void setLevelsStatus(int status);
    void setCurvesStatus(int status);
    
    QImage createFinalLightImage() const;
    QImage applyCurves(const QImage& image, int status) const;
    
    bool isDefaultCrop() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

signals:
    void emptyCanvasClicked();
    void toggleOrientationRequested();
    void forwardRequested();
    void backRequested();
    void lightKeyPressed(
        int key
    );
    void keyPressed(QKeyEvent* event);

private:
    enum class MouseMode
    {
        None,
        Move,
        Resize
    };

    enum class ResizeHandle
    {
        None,
        Left,
        Right,
        Top,
        Bottom,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    MouseMode m_mouseMode = MouseMode::None;
    ResizeHandle m_resizeHandle = ResizeHandle::None;
    QPointF m_lastMousePosition;

    QPointF m_resizeStartMousePosition;

    double m_resizeStartWidth = 0.0;
    double m_resizeStartHeight = 0.0;
    double m_resizeStartCenterX = 0.0;
    double m_resizeStartCenterY = 0.0;


    void updateCropFrame() const;

    QRectF imageRectOnScreen() const;
    QRectF cropRectOnScreen() const;

    void moveCrop();

    QImage m_image;

    double m_cropWidth;
    double m_cropHeight;

    double m_cropWidthCm = 10.0;
    double m_cropHeightCm = 15.0;

    // Положение центра рамки
    double m_cropCenterX;
    double m_cropCenterY;

    // Размер рамки в координатах исходного изображения
    double m_cropPixelWidth;
    double m_cropPixelHeight;

    static constexpr double MOVE_STEP_SCREEN = 5.3;

    QTimer* m_moveTimer;

    bool m_leftPressed;
    bool m_rightPressed;
    bool m_upPressed;
    bool m_downPressed;

    bool m_allowOverflowLeft;
    bool m_allowOverflowRight;
    bool m_allowOverflowTop;
    bool m_allowOverflowBottom;

    void updateScaledImage();
    QImage m_scaledImage;

    double imageToScreenScale() const;

    double m_overflowLeft;
    double m_overflowRight;
    double m_overflowTop;
    double m_overflowBottom;

    static constexpr double MAX_OVERFLOW_MM = 2.0;

    double maxOverflowScreenPixels(bool horizontal) const;
    double screenToImagePixels(double screenPixels) const;

    bool m_showCropResult = false;

    ResizeHandle resizeHandleAt(
        const QPointF& position
    ) const;

    bool isInsideCropRect(
        const QPointF& position
    ) const;

    QRectF constrainResizeRect(
        const QRectF& rect
    ) const;

    double limitResizeWidth(
        ResizeHandle handle,
        double desiredWidth
    ) const;

    bool m_fitImageMode = false;
    bool m_fitImageWithMargins = false;

    QImage applyLevels(
        const QImage& image,
        int status
    ) const;
    std::array<uchar, 256> createCurvesLut(int status) const;

    LightSettings m_lightSettings;

    QImage m_lightSourceImage;
    QImage m_lightPreviewSource;

    static constexpr int MIN_LIGHT_STATUS = -3;
    static constexpr int MAX_LIGHT_STATUS = 3;

    void updateLightPreview();

    QImage applyLightEffects(
        const QImage& image
    ) const;

    std::array<uchar, 256> createLevelsLut(
        int status
    ) const;
    
    
};

