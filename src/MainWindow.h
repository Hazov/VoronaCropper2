#pragma once

#include <QMainWindow>
#include <QStringList>
#include "ImageItem.h"

class QLabel;
class QLineEdit;
class CropCanvas;
#include <QPixmap>
#include <QVector>
#include <QPushButton>
#include <QProgressBar>

class QKeyEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void markCurrentPhoto();

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
        Light,
        Save
    };
    
    enum class SaveSourceType
    {
        Directory,
        SingleFile,
        MultipleFiles
    };
    
    enum class LightParameter
    {
        Levels,
        Curves
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
    
    SaveSourceType saveSourceType =
    SaveSourceType::MultipleFiles;

    QString sourceDirectory;
    QString saveHash;   
    
    void choosePhotosFromDialog();
    
    void applyCropSize() const;
    
    void toggleCropOrientation();
    
    QVector<QPixmap> levelsPixmaps;
    QVector<QPixmap> curvesPixmaps;
    
    int lightStatusToIndex(int status) const;
    
    QLabel *stageTitleLabel;

    QLabel *curvesImageLabel;
    QLabel *levelsImageLabel;

    QLabel *levelsTextLabel;
    QLabel *curvesTextLabel;

    QWidget *stageHintWidget;
    
    void loadLightStatusImages();
    
    void updateLightStatusWidgets();
    void updateStageHint();
    QWidget *lightStatusWidget;
    QLabel *keysImageLabel;
    
    QLabel *movementLabel;
    
    bool m_fitImageMode = false;
    bool m_fitImageWithMargins = false;
    
    
    void handleLightKey(
    QKeyEvent* event
    );
    
    QPushButton *saveButton;
    void saveAllImages();
    
    QString getSaveDirectory() const;
    QString getSaveFilePath(const ImageItem& item) const;
    
    bool saveImage(const ImageItem& item);

    QString getDesktopPath() const;
    
    QString makeUniquePath(const QString& path) const;
    QString saveDirectoryPath;
    
    void showDesktop();
    
    QWidget *imageContainer;
    
    QProgressBar *saveProgressBar;
    
};