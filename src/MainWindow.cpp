#include "MainWindow.h"
#include "CropCanvas.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QDirIterator>
#include <QMessageBox>
#include <QImageReader>
#include <QFileDialog>
#include <algorithm>
#include <QDoubleValidator>
#include "ImageItem.h"
#include <QVector>
#include <QCoreApplication>
#include <QDir>
#include <QFile>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      photoCounterLabel(nullptr),
      loadNewPhotosLabel(nullptr),
      logoLabel(nullptr),
      widthEdit(nullptr),
      heightEdit(nullptr),
      hintLabel(nullptr),
      cropCanvas(nullptr),
      backLabel(nullptr),
      markLabel(nullptr),
      forwardLabel(nullptr),
      currentImageIndex(-1),
      movementLabel(nullptr)
{
    setAcceptDrops(true);

    setupUi();

    loadLightStatusImages();

    setupWindowSize();

    connect(
        cropCanvas,
        &CropCanvas::emptyCanvasClicked,
        this,
        &MainWindow::choosePhotosFromDialog
    );

    connect(
        cropCanvas,
        &CropCanvas::toggleOrientationRequested,
        this,
        &MainWindow::toggleCropOrientation
    );

    connect(
        cropCanvas,
        &CropCanvas::forwardRequested,
        this,
        &MainWindow::goForward
    );

    connect(
        cropCanvas,
        &CropCanvas::backRequested,
        this,
        &MainWindow::goBack
    );
}


void MainWindow::setupUi()
{
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* mainLayout = new QVBoxLayout(centralWidget);

    mainLayout->setContentsMargins(25, 15, 25, 15);
    mainLayout->setSpacing(10);


    // =========================================================
    // HEADER
    // =========================================================

    auto* headerLayout = new QHBoxLayout();

    // ---------------------------------------------------------
    // Левая часть header
    // ---------------------------------------------------------

    auto* leftHeaderLayout = new QVBoxLayout();

    leftHeaderLayout->setSpacing(2);

    photoCounterLabel =
        new QLabel("Фото 0 из 0", this);

    loadNewPhotosLabel =
        new QLabel("Загрузить новые фото", this);

    loadNewPhotosLabel->setText(
        "<a href=\"#\">Загрузить новые фото</a>"
    );

    loadNewPhotosLabel->setTextFormat(Qt::RichText);

    loadNewPhotosLabel->setText(
        "<a href=\"#\">Загрузить новые фото</a>"
    );

    loadNewPhotosLabel->setCursor(
        Qt::PointingHandCursor
    );

    connect(
        loadNewPhotosLabel,
        &QLabel::linkActivated,
        this,
        &MainWindow::requestNewPhotos
    );

    // Пока фотографий нет — ссылка скрыта.
    loadNewPhotosLabel->hide();

    leftHeaderLayout->addWidget(
        photoCounterLabel
    );

    leftHeaderLayout->addWidget(
        loadNewPhotosLabel
    );


    // ---------------------------------------------------------
    // Название
    // ---------------------------------------------------------

    auto* titleLabel =
        new QLabel("Быстрое кадрирование", this);

    titleLabel->setAlignment(
        Qt::AlignCenter
    );

    QFont titleFont =
        titleLabel->font();

    titleFont.setPointSize(16);
    titleFont.setBold(true);

    titleLabel->setFont(titleFont);


    // ---------------------------------------------------------
    // Логотип
    // ---------------------------------------------------------

    logoLabel =
        new QLabel("LOGO", this);

    logoLabel->setAlignment(
        Qt::AlignCenter
    );

    logoLabel->setFixedSize(
        80,
        40
    );


    // ---------------------------------------------------------
    // Собираем header
    // ---------------------------------------------------------

    headerLayout->addLayout(
        leftHeaderLayout
    );

    headerLayout->addStretch();

    headerLayout->addWidget(
        titleLabel
    );

    headerLayout->addStretch();

    headerLayout->addWidget(
        logoLabel
    );

    mainLayout->addLayout(
        headerLayout
    );


    // =========================================================
    // РАЗМЕРЫ КАДРИРОВАНИЯ
    // =========================================================

    auto* settingsLayout =
        new QHBoxLayout();

    auto* widthLabel =
        new QLabel("Ширина:", this);

    auto* heightLabel =
        new QLabel("Высота:", this);

    widthEdit =
        new QLineEdit("10", this);

    heightEdit =
        new QLineEdit("15", this);

    widthEdit->setFixedWidth(70);
    heightEdit->setFixedWidth(70);

    auto* sizeValidator =
        new QDoubleValidator(
            1.0,
            1000.0,
            2,
            this
        );

    sizeValidator->setNotation(
        QDoubleValidator::StandardNotation
    );

    sizeValidator->setLocale(
        QLocale::c()
    );

    widthEdit->setValidator(sizeValidator);
    heightEdit->setValidator(sizeValidator);


    connect(
        widthEdit,
        &QLineEdit::textChanged,
        this,
        &MainWindow::applyCropSize
    );

    connect(
        heightEdit,
        &QLineEdit::textChanged,
        this,
        &MainWindow::applyCropSize
    );

    settingsLayout->addStretch();

    settingsLayout->addWidget(
        widthLabel
    );

    settingsLayout->addWidget(
        widthEdit
    );

    settingsLayout->addWidget(
        new QLabel("см", this)
    );

    settingsLayout->addSpacing(25);

    settingsLayout->addWidget(
        heightLabel
    );

    settingsLayout->addWidget(
        heightEdit
    );

    settingsLayout->addWidget(
        new QLabel("см", this)
    );

    settingsLayout->addStretch();

    mainLayout->addLayout(
        settingsLayout
    );


    // =========================================================
    // ПОДСКАЗКА СТАДИИ
    // =========================================================

    stageHintWidget = new QWidget(this);

    QVBoxLayout* stageHintLayout =
        new QVBoxLayout(stageHintWidget);

    stageHintLayout->setContentsMargins(
        0, 0, 0, 0
    );

    stageHintLayout->setSpacing(6);

    stageTitleLabel =
        new QLabel(
            "Настройка рамки",
            stageHintWidget
        );

    stageTitleLabel->setAlignment(
        Qt::AlignCenter
    );

    stageHintLayout->addWidget(
        stageTitleLabel
    );

    keysImageLabel =
        new QLabel(stageHintWidget);

    keysImageLabel->setAlignment(
        Qt::AlignCenter
    );

    keysImageLabel->setPixmap(
        QPixmap(
            "light/keys.png"
        )
    );

    stageHintLayout->addWidget(
        keysImageLabel
    );
    
    movementLabel =
    new QLabel(
        "Движение рамки",
        stageHintWidget
    );

    movementLabel->setAlignment(
        Qt::AlignCenter
    );

    stageHintLayout->addWidget(
        movementLabel
    );

    lightStatusWidget =
        new QWidget(stageHintWidget);
    
    lightStatusWidget->hide();

    QGridLayout* lightGrid =
        new QGridLayout(lightStatusWidget);

    lightGrid->setContentsMargins(
        0, 0, 0, 0
    );

    lightGrid->setHorizontalSpacing(20);
    lightGrid->setVerticalSpacing(4);

    levelsImageLabel =
        new QLabel(lightStatusWidget);

    levelsImageLabel->setAlignment(
        Qt::AlignCenter
    );

    lightGrid->addWidget(
        levelsImageLabel,
        0, 0
    );

    curvesImageLabel =
        new QLabel(lightStatusWidget);

    curvesImageLabel->setAlignment(
        Qt::AlignCenter
    );

    lightGrid->addWidget(
        curvesImageLabel,
        0, 1
    );

    levelsTextLabel =
        new QLabel(
            "Уровни",
            lightStatusWidget
        );

    levelsTextLabel->setAlignment(
        Qt::AlignCenter
    );

    lightGrid->addWidget(
        levelsTextLabel,
        1, 0
    );

    curvesTextLabel =
        new QLabel(
            "Кривые",
            lightStatusWidget
        );

    curvesTextLabel->setAlignment(
        Qt::AlignCenter
    );

    lightGrid->addWidget(
        curvesTextLabel,
        1, 1
    );

    stageHintLayout->addWidget(
        lightStatusWidget
    );
    
    mainLayout->addWidget(
    stageHintWidget
);


    // =========================================================
    // IMAGE CANVAS
    // =========================================================

    auto* imageContainer =
        new QWidget(this);

    auto* imageLayout =
        new QVBoxLayout(imageContainer);

    imageContainer->setStyleSheet(
        "background-color: #808080;"
    );

    imageLayout->setContentsMargins(
        30,
        10,
        30,
        20
    );

    imageLayout->setSpacing(0);

    cropCanvas =
        new CropCanvas(this);

    imageLayout->addWidget(
        cropCanvas
    );

    mainLayout->addWidget(
        imageContainer,
        1
    );


    // =========================================================
    // FOOTER
    // =========================================================

    auto* footerLayout =
        new QHBoxLayout();

    backLabel =
        new QLabel(this);

    markLabel =
        new QLabel(this);

    forwardLabel =
        new QLabel(this);


    backLabel->setText(
        "<a href=\"#\">← Назад</a>"
    );

    markLabel->setText(
        "<a href=\"#\">Пометить</a>"
    );

    forwardLabel->setText(
        "<a href=\"#\">Вперёд →</a>"
    );


    backLabel->setTextFormat(
        Qt::RichText
    );

    markLabel->setTextFormat(
        Qt::RichText
    );

    forwardLabel->setTextFormat(
        Qt::RichText
    );


    backLabel->setCursor(
        Qt::PointingHandCursor
    );

    markLabel->setCursor(
        Qt::PointingHandCursor
    );

    forwardLabel->setCursor(
        Qt::PointingHandCursor
    );

    connect(
        backLabel,
        &QLabel::linkActivated,
        this,
        &MainWindow::goBack
    );

    connect(
        forwardLabel,
        &QLabel::linkActivated,
        this,
        &MainWindow::goForward
    );


    footerLayout->addStretch();

    footerLayout->addWidget(
        backLabel
    );

    footerLayout->addSpacing(35);

    footerLayout->addWidget(
        markLabel
    );

    footerLayout->addSpacing(35);

    footerLayout->addWidget(
        forwardLabel
    );

    footerLayout->addStretch();

    mainLayout->addLayout(
        footerLayout
    );
}


void MainWindow::setupWindowSize()
{
    QScreen* screen = QGuiApplication::primaryScreen();

    if (!screen)
        return;

    const QRect availableGeometry = screen->availableGeometry();

    const int width =
        static_cast<int>(availableGeometry.width() * 0.70);

    const int height =
        static_cast<int>(availableGeometry.height() * 0.90);

    setFixedSize(width, height);

    const int x =
        availableGeometry.x()
        + (availableGeometry.width() - width) / 2;

    const int y =
        availableGeometry.y()
        + (availableGeometry.height() - height) / 2;

    move(x, y);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (!images.isEmpty())
    {
        event->ignore();
        return;
    }

    if (!event->mimeData()->hasUrls())
    {
        event->ignore();
        return;
    }

    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* event)
{
    if (!images.isEmpty())
    {
        event->ignore();
        return;
    }

    if (!event->mimeData()->hasUrls())
    {
        event->ignore();
        return;
    }

    const QList<QUrl> urls =
        event->mimeData()->urls();

    loadDroppedUrls(urls);

    event->acceptProposedAction();
}

void MainWindow::loadImageFiles(const QStringList& files)
{
    images.clear();

    for (const QString& filePath : files)
    {
        ImageItem item(filePath);

        if (item.isValid())
        {
            images.append(item);
        }
    }

    currentImageIndex = 0;

    loadNewPhotosLabel->show();

    showCurrentImage();
}

bool MainWindow::isImageFile(const QString& filePath) const
{
    const QList<QByteArray> supportedFormats =
        QImageReader::supportedImageFormats();

    const QString suffix =
        QFileInfo(filePath)
        .suffix()
        .toLower();

    for (const QByteArray& format : supportedFormats)
    {
        if (QString::fromLatin1(format).toLower() == suffix)
            return true;
    }

    return false;
}

void MainWindow::showCurrentImage()
{
    updateStageHint();
    
    if (currentImageIndex < 0 ||
        currentImageIndex >= images.size())
    {
        cropCanvas->setImage(QImage());

        photoCounterLabel->setText(
            "Фото 0 из 0"
        );

        loadNewPhotosLabel->hide();

        return;
    }

    ImageItem& item =
        images[currentImageIndex];

    const QImage& image =
        item.image();

    if (image.isNull())
    {
        QMessageBox::warning(
            this,
            "Ошибка загрузки",
            QString(
                "Не удалось открыть изображение:\n%1"
            )
            .arg(item.filePath())
        );

        return;
    }

    cropCanvas->setDisplayImage(
        image,
        currentStage == Stage::Light
    );

    // Определяем ориентацию фотографии.
    const double width =
        item.cropWidthCm();

    const double height =
        item.cropHeightCm();

    widthEdit->setText(
        QString::number(
            width,
            'g',
            10
        )
    );

    heightEdit->setText(
        QString::number(
            height,
            'g',
            10
        )
    );

    cropCanvas->setCropRatio(
        width,
        height
    );

    cropCanvas->setCropCenter(
        item.cropCenterX(),
        item.cropCenterY()
    );

    photoCounterLabel->setText(
        QString("Фото %1 из %2")
        .arg(currentImageIndex + 1)
        .arg(images.size())
    );

    loadNewPhotosLabel->show();

    setWindowTitle(
        QString("VoronaCropper — %1")
        .arg(
            QFileInfo(
                item.filePath()
            ).fileName()
        )
    );
}

void MainWindow::loadDroppedUrls(const QList<QUrl>& urls)
{
    QStringList imageFiles;
    QStringList unsupportedFiles;

    for (const QUrl& url : urls)
    {
        if (!url.isLocalFile())
            continue;

        const QString path = url.toLocalFile();
        QFileInfo info(path);

        if (info.isDir())
        {
            QDirIterator iterator(
                path,
                QDir::Files,
                QDirIterator::Subdirectories
            );

            while (iterator.hasNext())
            {
                const QString filePath = iterator.next();

                if (isImageFile(filePath))
                    imageFiles.append(filePath);
                else
                    unsupportedFiles.append(filePath);
            }
        }
        else if (info.isFile())
        {
            if (isImageFile(path))
                imageFiles.append(path);
            else
                unsupportedFiles.append(path);
        }
    }

    if (!unsupportedFiles.isEmpty())
    {
        const QString message =
            QString(
                "Некоторые файлы не являются "
                "поддерживаемыми изображениями.\n\n"
                "Таких файлов: %1\n\n"
                "Они будут проигнорированы. Продолжить?"
            ).arg(unsupportedFiles.size());

        const QMessageBox::StandardButton result =
            QMessageBox::warning(
                this,
                "Неподдерживаемые файлы",
                message,
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::Yes
            );

        if (result == QMessageBox::No)
            return;
    }

    if (imageFiles.isEmpty())
    {
        QMessageBox::information(
            this,
            "VoronaCropper",
            "Изображения не найдены."
        );

        return;
    }

    loadImageFiles(imageFiles);
}

void MainWindow::requestNewPhotos()
{
    const QMessageBox::StandardButton result =
        QMessageBox::question(
            this,
            "Загрузить новые фото",
            "Текущий набор фотографий будет удалён.\n\n"
            "Вы действительно хотите начать заново?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );

    if (result != QMessageBox::Yes)
        return;

    resetProject();
}

void MainWindow::resetProject()
{
    images.clear();

    currentImageIndex = -1;

    cropCanvas->clearImage();

    photoCounterLabel->setText(
        "Фото 0 из 0"
    );

    loadNewPhotosLabel->hide();

    setWindowTitle(
        "VoronaCropper"
    );
}

void MainWindow::choosePhotosFromDialog()
{
    if (!images.isEmpty())
        return;

    const QStringList files =
        QFileDialog::getOpenFileNames(
            this,
            "Выберите фотографии",
            QString(),
            "Изображения (*.png *.jpg *.jpeg *.bmp *.webp *.heic *.heif)"
        );

    if (files.isEmpty())
        return;

    loadImageFiles(files);
}

void MainWindow::applyCropSize() const
{
    bool widthOk = false;
    bool heightOk = false;

    const QString widthText =
        widthEdit->text().replace(',', '.');

    const QString heightText =
        heightEdit->text().replace(',', '.');

    const double width =
        widthText.toDouble(&widthOk);

    const double height =
        heightText.toDouble(&heightOk);

    // Пока пользователь вводит число,
    // которое ещё не является полноценным значением,
    // ничего не меняем.
    if (!widthOk || !heightOk)
        return;

    const double clampedWidth =
        std::clamp(width, 1.0, 1000.0);

    const double clampedHeight =
        std::clamp(height, 1.0, 1000.0);

    if (clampedWidth != width)
    {
        widthEdit->blockSignals(true);

        widthEdit->setText(
            QString::number(
                clampedWidth,
                'g',
                10
            )
        );

        widthEdit->blockSignals(false);
    }

    if (clampedHeight != height)
    {
        heightEdit->blockSignals(true);

        heightEdit->setText(
            QString::number(
                clampedHeight,
                'g',
                10
            )
        );

        heightEdit->blockSignals(false);
    }

    cropCanvas->setCropRatio(
        clampedWidth,
        clampedHeight
    );
}

void MainWindow::toggleCropOrientation()
{
    const QString widthText =
        widthEdit->text();

    const QString heightText =
        heightEdit->text();

    widthEdit->setText(heightText);
    heightEdit->setText(widthText);

    applyCropSize();
}

void MainWindow::saveCurrentCropState()
{
    if (currentImageIndex < 0 ||
        currentImageIndex >= images.size())
    {
        return;
    }

    ImageItem& item =
        images[currentImageIndex];

    const double width =
        widthEdit->text()
                 .replace(',', '.')
                 .toDouble();

    const double height =
        heightEdit->text()
                  .replace(',', '.')
                  .toDouble();

    item.setCropSize(
        width,
        height
    );

    item.setCropCenter(
        cropCanvas->cropCenterX(),
        cropCanvas->cropCenterY()
    );

    item.setCropRect(
        cropCanvas->cropRectInImage()
    );
}

void MainWindow::showPreviousImage()
{
    if (images.isEmpty())
        return;

    if (currentImageIndex <= 0)
        return;

    saveCurrentCropState();

    --currentImageIndex;

    showCurrentImage();
}

void MainWindow::showNextImage()
{
    if (images.isEmpty())
        return;

    if (currentImageIndex >= images.size() - 1)
        return;

    saveCurrentCropState();

    ++currentImageIndex;

    showCurrentImage();
}

void MainWindow::goForward()
{
    if (images.isEmpty())
        return;

    if (currentStage == Stage::Crop)
    {
        saveCurrentCropState();

        ImageItem& item =
            images[currentImageIndex];

        item.createCroppedImage(
            item.cropRect()
        );

        currentStage = Stage::Light;
        
        updateStageHint();

        cropCanvas->setDisplayImage(
            item.croppedImage(),
            true
        );
        

        return;
    }

    // Stage::Light
    if (currentImageIndex >= images.size() - 1)
        return;

    ++currentImageIndex;

    currentStage = Stage::Crop;
    
    updateStageHint();

    showCurrentImage();
}

void MainWindow::goBack()
{
    if (images.isEmpty())
        return;

    // Свет → Кадр
    if (currentStage == Stage::Light)
    {
        currentStage = Stage::Crop;

        ImageItem& item =
            images[currentImageIndex];

        widthEdit->setText(
            QString::number(
                item.cropWidthCm(),
                'g',
                10
            )
        );

        heightEdit->setText(
            QString::number(
                item.cropHeightCm(),
                'g',
                10
            )
        );

        cropCanvas->setDisplayImage(
            item.originalImage(),
            false
        );

        cropCanvas->setCropRatio(
            item.cropWidthCm(),
            item.cropHeightCm()
        );

        const QRectF savedCropRect =
            item.cropRect();

        cropCanvas->setCropPixelSize(
            savedCropRect.width(),
            savedCropRect.height()
        );

        cropCanvas->setCropCenter(
            item.cropCenterX(),
            item.cropCenterY()
        );

        updateStageHint();

        return;
    }

    // Кадр → предыдущая фотография → Свет
    if (currentImageIndex <= 0)
        return;

    // Сохраняем состояние текущей фотографии.
    saveCurrentCropState();

    --currentImageIndex;

    ImageItem& item =
        images[currentImageIndex];

    // Создаём результат кадрирования
    // именно по состоянию этой фотографии.
    item.createCroppedImage(
        item.cropRect()
    );

    currentStage = Stage::Light;

    cropCanvas->setDisplayImage(
        item.croppedImage(),
        true
    );

    photoCounterLabel->setText(
        QString("Фото %1 из %2")
        .arg(currentImageIndex + 1)
        .arg(images.size())
    );

    updateStageHint();
}

void CropCanvas::mouseReleaseEvent(
    QMouseEvent* event)
{
    m_mouseMode = MouseMode::None;
    m_resizeHandle = ResizeHandle::None;

    setCursor(Qt::ArrowCursor);

    event->accept();
}

void MainWindow::loadLightStatusImages()
{
    levelsPixmaps.clear();
    curvesPixmaps.clear();

    levelsPixmaps.reserve(7);
    curvesPixmaps.reserve(7);

    for (int status = -3; status <= 3; ++status)
    {
        const QString suffix =
            status > 0
                ? QString("+%1").arg(status)
                : QString::number(status);

        const QString lightPath =
            QDir(
                QCoreApplication::applicationDirPath()
            ).filePath("light");

        const QString levelsPath =
            QDir(lightPath).filePath(
                QString("levels%1.jpg").arg(suffix)
            );

        const QString curvesPath =
            QDir(lightPath).filePath(
                QString("curves%1.jpg").arg(suffix)
            );

        const QPixmap levelsPixmap(
            levelsPath
        );

        const QPixmap curvesPixmap(
            curvesPath
        );

        levelsPixmaps.append(levelsPixmap);
        curvesPixmaps.append(curvesPixmap);
    }
}

int MainWindow::lightStatusToIndex(int status) const
{
    return status + 3;
}

void MainWindow::updateLightStatusWidgets()
{
    if (currentImageIndex < 0 ||
        currentImageIndex >= images.size())
    {
        return;
    }

    const ImageItem& item =
        images[currentImageIndex];

    const int levelsIndex =
        lightStatusToIndex(
            item.levelsStatus()
        );

    const int curvesIndex =
        lightStatusToIndex(
            item.curvesStatus()
        );

    levelsImageLabel->setPixmap(
        levelsPixmaps[levelsIndex]
    );

    curvesImageLabel->setPixmap(
        curvesPixmaps[curvesIndex]
    );
}

void MainWindow::updateStageHint()
{
    if (currentStage == Stage::Crop)
    {
        stageTitleLabel->setText(
            "Настройка рамки"
        );

        keysImageLabel->show();
        movementLabel->show();
        lightStatusWidget->hide();
    }
    else
    {
        stageTitleLabel->setText(
            "Настройка света"
        );

        keysImageLabel->hide();
        movementLabel->hide();
        lightStatusWidget->show();

        updateLightStatusWidgets();
    }
}
