#pragma once

#include <QImage>
#include <QRectF>

class ImageItem
{
public:
    ImageItem() = default;

    explicit ImageItem(const QString& filePath);

    bool isValid() const;

    const QImage& image() const;

    const QImage& originalImage() const;
    const QImage& croppedImage() const;

    void setCroppedImage(
        const QImage& image
    );

    void createCroppedImage(const QRectF& cropRect);
    void createCroppedImage();

    const QString& filePath() const;

    double cropWidthCm() const;
    double cropHeightCm() const;

    double cropCenterX() const;
    double cropCenterY() const;

    void setCropSize(double widthCm, double heightCm);

    void setCropCenter(double x, double y);

    QRectF cropRect() const;

    void setCropRect(const QRectF& rect);

    int levelsStatus() const;
    int curvesStatus() const;

    void setLevelsStatus(int value);
    void setCurvesStatus(int value);

    bool m_fitImageMode = false;
    bool m_fitImageWithMargins = false;

    void setFitImageMode(
        bool enabled,
        bool withMargins
    );

    bool fitImageMode() const;
    bool fitImageWithMargins() const;

    void createFittedCroppedImage(
        const QRectF& cropRect,
        bool withMargins
    );
    
    bool isMarked() const;
    void setMarked(bool marked);

private:
    QString m_filePath;
    QImage m_originalImage;
    QImage m_croppedImage;

    // Состояние стадии «Кадр»
    double m_cropWidthCm = 10.0;
    double m_cropHeightCm = 15.0;

    // Положение центра рамки
    // в координатах исходного изображения.
    double m_cropCenterX = 0.0;
    double m_cropCenterY = 0.0;
    QRectF m_cropRect;

    int m_levelsStatus = 0;
    int m_curvesStatus = 0;

    static constexpr int MIN_LIGHT_STATUS = -3;
    static constexpr int MAX_LIGHT_STATUS = 3;

    bool m_marked = false;
};
