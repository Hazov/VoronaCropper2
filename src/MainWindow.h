#pragma once

#include <QMainWindow>
#include <QStringList>
#include "ImageItem.h"

class QLabel;
class QLineEdit;
class CropCanvas;
#include <QPixmap>
#include <QVector>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    void setupUi();
    void setupWindowSize();

    // Работа с файлами
    void loadDroppedUrls(const QList<QUrl> &urls);
    void loadImageFiles(const QStringList &files);
    void showCurrentImage();
    
    void showPreviousImage();
    void showNextImage();

    void saveCurrentCropState();
    
    void goForward();
    void goBack();

    bool isImageFile(const QString &filePath) const;

    // Управление текущим набором фотографий
    void resetProject();
    void requestNewPhotos();
    
    enum class Stage
    {
        Crop,
        Light
    };

    Stage currentStage = Stage::Crop;

    // Header
    QLabel *photoCounterLabel;
    QLabel *loadNewPhotosLabel;
    QLabel *logoLabel;

    // Settings
    QLineEdit *widthEdit;
    QLineEdit *heightEdit;

    // Stage hint
    QLabel *hintLabel;

    // Main image
    CropCanvas *cropCanvas;

    // Footer
    QLabel *backLabel;
    QLabel *markLabel;
    QLabel *forwardLabel;

    // Текущий набор фотографий
    QVector<ImageItem> images;
    int currentImageIndex;
    
    void choosePhotosFromDialog();
    
    void applyCropSize() const;
    
    void toggleCropOrientation();
    
    QVector<QPixmap> levelsPixmaps;
    QVector<QPixmap> curvesPixmaps;
    
    int lightStatusToIndex(int status) const;
    
    QLabel *stageTitleLabel;

    QLabel *levelsImageLabel;
    QLabel *curvesImageLabel;

    QLabel *levelsTextLabel;
    QLabel *curvesTextLabel;

    QWidget *stageHintWidget;
    
    void loadLightStatusImages();
    
    void updateLightStatusWidgets();
    void updateStageHint();
    QWidget *lightStatusWidget;
    QLabel *keysImageLabel;
    
    QLabel *movementLabel;
    
    
};