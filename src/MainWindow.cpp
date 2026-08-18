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
#include <QFileInfo>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <windows.h>
#include <shldisp.h>
#include <QKeyEvent>
#include <QApplication>

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
      saveButton(nullptr),
      currentImageIndex(-1),
      imageContainer(nullptr),
      movementLabel(nullptr),
      saveProgressBar(nullptr)

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

    connect(
        cropCanvas,
        &CropCanvas::keyPressed,
        this,
        &MainWindow::handleLightKey
    );
}


void MainWindow::setupUi()
{
    setWindowTitle(
        "Быстрое кадрирование"
    );

    auto* centralWidget =
        new QWidget(this);

    setCentralWidget(
        centralWidget
    );

    auto* mainLayout =
        new QVBoxLayout(
            centralWidget
        );

    mainLayout->setContentsMargins(
        25,
        15,
        25,
        15
    );

    mainLayout->setSpacing(10);


    // =========================================================
    // HEADER
    // =========================================================

    auto* headerWidget =
        new QWidget(this);

    headerWidget->setFixedHeight(80);

    auto* headerLayout =
        new QGridLayout(headerWidget);

    headerLayout->setContentsMargins(
        0,
        0,
        0,
        0
    );


    // =========================================================
    // ЛЕВАЯ ЧАСТЬ
    // =========================================================

    auto* leftHeaderWidget =
        new QWidget(headerWidget);

    auto* leftHeaderLayout =
        new QVBoxLayout(leftHeaderWidget);

    leftHeaderLayout->setContentsMargins(
        0,
        0,
        0,
        0
    );

    leftHeaderLayout->setSpacing(2);


    photoCounterLabel =
        new QLabel(
            "Быстрое кадрирование",
            leftHeaderWidget
        );

    photoCounterLabel->setAlignment(
        Qt::AlignLeft
    );

    QFont titleFont =
        photoCounterLabel->font();

    titleFont.setPointSize(16);
    titleFont.setBold(true);

    photoCounterLabel->setFont(
        titleFont
    );


    loadNewPhotosLabel =
        new QLabel(
            "Загрузить новые фото",
            leftHeaderWidget
        );

    loadNewPhotosLabel->setText(
        "<a href=\"#\">Загрузить новые фото</a>"
    );

    loadNewPhotosLabel->setTextFormat(
        Qt::RichText
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

    loadNewPhotosLabel->hide();


    leftHeaderLayout->addWidget(
        photoCounterLabel
    );

    leftHeaderLayout->addWidget(
        loadNewPhotosLabel
    );


    // =========================================================
    // ЦЕНТР — РАЗМЕРЫ
    // =========================================================

    auto* sizeWidget =
        new QWidget(headerWidget);

    auto* sizeLayout =
        new QHBoxLayout(sizeWidget);

    sizeLayout->setContentsMargins(
        0,
        0,
        0,
        0
    );

    sizeLayout->setSpacing(5);


    auto* widthLabel =
        new QLabel(
            "Ширина:",
            sizeWidget
        );

    auto* heightLabel =
        new QLabel(
            "Высота:",
            sizeWidget
        );


    widthEdit =
        new QLineEdit(
            "10",
            sizeWidget
        );

    heightEdit =
        new QLineEdit(
            "15",
            sizeWidget
        );


    widthEdit->setFixedWidth(70);
    heightEdit->setFixedWidth(70);


    connect(
        widthEdit,
        &QLineEdit::returnPressed,
        this,
        &MainWindow::focusImage
    );

    connect(
        heightEdit,
        &QLineEdit::returnPressed,
        this,
        &MainWindow::focusImage
    );


    connect(
        widthEdit,
        &QLineEdit::textChanged,
        this,
        [this]()
        {
            normalizeSizeInput(widthEdit);
            applyCropSize();
        }
    );

    connect(
        heightEdit,
        &QLineEdit::textChanged,
        this,
        [this]()
        {
            normalizeSizeInput(heightEdit);
            applyCropSize();
        }
    );


    sizeLayout->addWidget(
        widthLabel
    );

    sizeLayout->addWidget(
        widthEdit
    );

    sizeLayout->addWidget(
        new QLabel(
            "см",
            sizeWidget
        )
    );


    sizeLayout->addSpacing(25);


    sizeLayout->addWidget(
        heightLabel
    );

    sizeLayout->addWidget(
        heightEdit
    );

    sizeLayout->addWidget(
        new QLabel(
            "см",
            sizeWidget
        )
    );


    // =========================================================
    // ПРАВАЯ ЧАСТЬ — LOGO
    // =========================================================

    logoLabel =
        new QLabel(headerWidget);

    logoLabel->setAlignment(
        Qt::AlignCenter
    );


    const QString logoPath =
        QDir(
            QCoreApplication::applicationDirPath()
        ).filePath(
            "logo.png"
        );


    QPixmap logoPixmap(
        logoPath
    );


    logoLabel->setPixmap(
        logoPixmap.scaled(
            80,
            80,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        )
    );


    logoLabel->setFixedSize(
        80,
        80
    );


    // =========================================================
    // РАСКЛАДКА HEADER
    // =========================================================

    headerLayout->addWidget(
        leftHeaderWidget,
        0,
        0,
        Qt::AlignLeft | Qt::AlignTop
    );


    headerLayout->addWidget(
        sizeWidget,
        0,
        1,
        Qt::AlignCenter
    );


    headerLayout->addWidget(
        logoLabel,
        0,
        2,
        Qt::AlignRight | Qt::AlignTop
    );


    // Левая и правая колонки одинаковые.
    // Поэтому центральный блок реально находится по центру.
    headerLayout->setColumnStretch(
        0,
        1
    );

    headerLayout->setColumnStretch(
        1,
        0
    );

    headerLayout->setColumnStretch(
        2,
        1
    );


    mainLayout->addWidget(
        headerWidget
    );


    // =========================================================
    // ПОДСКАЗКА СТАДИИ
    // =========================================================

    stageHintWidget =
        new QWidget(this);

    QVBoxLayout* stageHintLayout =
        new QVBoxLayout(stageHintWidget);

    stageHintLayout->setContentsMargins(
        0,
        0,
        0,
        0
    );

    stageHintLayout->setSpacing(6);


    // -------------------------
    // Заголовок подсказки
    // -------------------------

    stageTitleLabel =
        new QLabel(stageHintWidget);

    stageTitleLabel->setAlignment(
        Qt::AlignCenter
    );

    stageHintLayout->addWidget(
        stageTitleLabel
    );


    // -------------------------
    // Три подсказки горизонтально
    // -------------------------

    cropHintsWidget =
        new QWidget(stageHintWidget);

    QHBoxLayout* cropHintsLayout =
        new QHBoxLayout(cropHintsWidget);

    stageHintLayout->addWidget(cropHintsWidget);


    cropHintsLayout->setSpacing(50);

    cropHintsLayout->setAlignment(
        Qt::AlignCenter
    );


    // =========================
    // ESC
    // =========================

    QVBoxLayout* cropEscHintLayout =
        new QVBoxLayout();

    cropEscHintLayout->setAlignment(
        Qt::AlignCenter
    );

    cropEscHintLayout->setSpacing(2);
    cropEscHintLayout->setContentsMargins(0, 0, 0, 0);


    escImageLabel =
        new QLabel(stageHintWidget);

    escImageLabel->setAlignment(
        Qt::AlignCenter
    );

    escImageLabel->setPixmap(
        QPixmap(
            "light/esc_key.png"
        )
    );


    escLabel =
        new QLabel(
            "Сбросить рамку",
            stageHintWidget
        );

    escLabel->setAlignment(
        Qt::AlignCenter
    );


    cropEscHintLayout->addWidget(
        escImageLabel
    );

    cropEscHintLayout->addWidget(
        escLabel
    );


    cropHintsLayout->addLayout(
        cropEscHintLayout
    );


    // =========================
    // Стрелки
    // =========================

    QVBoxLayout* cropArrowsHintLayout =
        new QVBoxLayout();

    cropArrowsHintLayout->setAlignment(
        Qt::AlignCenter
    );

    cropArrowsHintLayout->setSpacing(2);
    cropArrowsHintLayout->setContentsMargins(0, 0, 0, 0);


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


    movementLabel =
        new QLabel(
            "Движение рамки / поле Epson",
            stageHintWidget
        );

    movementLabel->setAlignment(
        Qt::AlignCenter
    );


    cropHintsWidget->hide();


    cropArrowsHintLayout->addWidget(
        keysImageLabel
    );

    cropArrowsHintLayout->addWidget(
        movementLabel
    );


    cropHintsLayout->addLayout(
        cropArrowsHintLayout
    );

    // =========================
    // Shift
    // =========================

    QVBoxLayout* cropShiftHintLayout =
        new QVBoxLayout();

    cropShiftHintLayout->setAlignment(
        Qt::AlignCenter
    );

    cropShiftHintLayout->setSpacing(2);
    cropShiftHintLayout->setContentsMargins(0, 0, 0, 0);


    shiftImageLabel =
        new QLabel(stageHintWidget);

    shiftImageLabel->setAlignment(
        Qt::AlignCenter
    );

    shiftImageLabel->setPixmap(
        QPixmap(
            "light/shift_key.png"
        )
    );


    shiftLabel =
        new QLabel(
            "По размеру рамки",
            stageHintWidget
        );

    shiftLabel->setAlignment(
        Qt::AlignCenter
    );


    cropShiftHintLayout->addWidget(
        shiftImageLabel
    );

    cropShiftHintLayout->addWidget(
        shiftLabel
    );


    cropHintsLayout->addLayout(
        cropShiftHintLayout
    );

    // =========================
    // X
    // =========================

    QVBoxLayout* cropXHintLayout =
        new QVBoxLayout();

    cropXHintLayout->setAlignment(
        Qt::AlignCenter
    );

    cropXHintLayout->setSpacing(2);
    cropXHintLayout->setContentsMargins(0, 0, 0, 0);


    xImageLabel =
        new QLabel(stageHintWidget);

    xImageLabel->setAlignment(
        Qt::AlignCenter
    );

    xImageLabel->setPixmap(
        QPixmap(
            "light/x_key.png"
        )
    );


    xLabel =
        new QLabel(
            "Ориентация",
            stageHintWidget
        );

    xLabel->setAlignment(
        Qt::AlignCenter
    );


    cropXHintLayout->addWidget(
        xImageLabel
    );

    cropXHintLayout->addWidget(
        xLabel
    );


    cropHintsLayout->addLayout(
        cropXHintLayout
    );


    // =========================
    // Блок Light
    // =========================

    lightStatusWidget =
        new QWidget(
            stageHintWidget
        );

    lightStatusWidget->hide();


    QHBoxLayout* lightMainLayout =
        new QHBoxLayout(lightStatusWidget);

    lightMainLayout->setContentsMargins(
        0,
        0,
        0,
        0
    );

    lightMainLayout->setSpacing(50);

    lightMainLayout->setAlignment(
        Qt::AlignCenter
    );


    // =========================================================
    // Левая ячейка: left_key + curves + right_key
    // =========================================================

    QVBoxLayout* curvesHintLayout =
        new QVBoxLayout();

    curvesHintLayout->setAlignment(
        Qt::AlignCenter
    );

    curvesHintLayout->setSpacing(2);
    curvesHintLayout->setContentsMargins(0, 0, 0, 0);


    QHBoxLayout* curvesKeysLayout =
        new QHBoxLayout();

    curvesKeysLayout->setAlignment(
        Qt::AlignCenter
    );


    QLabel* leftKeyImageLabel =
        new QLabel(lightStatusWidget);

    leftKeyImageLabel->setAlignment(
        Qt::AlignCenter
    );

    leftKeyImageLabel->setPixmap(
        QPixmap("light/left_key.png")
    );


    curvesImageLabel =
        new QLabel(lightStatusWidget);

    curvesImageLabel->setAlignment(
        Qt::AlignCenter
    );


    QLabel* rightKeyImageLabel =
        new QLabel(lightStatusWidget);

    rightKeyImageLabel->setAlignment(
        Qt::AlignCenter
    );

    rightKeyImageLabel->setPixmap(
        QPixmap("light/right_key.png")
    );


    curvesKeysLayout->addWidget(
        leftKeyImageLabel
    );

    curvesKeysLayout->addWidget(
        curvesImageLabel
    );

    curvesKeysLayout->addWidget(
        rightKeyImageLabel
    );


    curvesTextLabel =
        new QLabel(
            "Уровни",
            lightStatusWidget
        );

    curvesTextLabel->setAlignment(
        Qt::AlignCenter
    );


    curvesHintLayout->addLayout(
        curvesKeysLayout
    );

    curvesHintLayout->addWidget(
        curvesTextLabel
    );


    // =========================================================
    // Правая ячейка: levels + vert_keys
    // =========================================================

    QVBoxLayout* levelsHintLayout =
        new QVBoxLayout();

    levelsHintLayout->setAlignment(
        Qt::AlignCenter
    );

    levelsHintLayout->setSpacing(2);
    levelsHintLayout->setContentsMargins(0, 0, 0, 0);


    QHBoxLayout* levelsKeysLayout =
        new QHBoxLayout();

    levelsKeysLayout->setAlignment(
        Qt::AlignCenter
    );


    levelsImageLabel =
        new QLabel(lightStatusWidget);

    levelsImageLabel->setAlignment(
        Qt::AlignCenter
    );


    QLabel* vertKeysImageLabel =
        new QLabel(lightStatusWidget);

    vertKeysImageLabel->setAlignment(
        Qt::AlignCenter
    );

    vertKeysImageLabel->setPixmap(
        QPixmap("light/vert_keys.png")
    );


    levelsKeysLayout->addWidget(
        levelsImageLabel
    );

    levelsKeysLayout->addWidget(
        vertKeysImageLabel
    );


    levelsTextLabel =
        new QLabel(
            "Кривые",
            lightStatusWidget
        );

    levelsTextLabel->setAlignment(
        Qt::AlignCenter
    );


    levelsHintLayout->addLayout(
        levelsKeysLayout
    );

    levelsHintLayout->addWidget(
        levelsTextLabel
    );


    // =========================================================
    // Добавляем две ячейки горизонтально
    // =========================================================

    lightMainLayout->addLayout(
        curvesHintLayout
    );

    lightMainLayout->addLayout(
        levelsHintLayout
    );


    stageHintLayout->addWidget(
        lightStatusWidget
    );


    // Добавляем блок подсказок в основной layout
    mainLayout->addWidget(
        stageHintWidget
    );


    // =========================================================
    // IMAGE CANVAS
    // =========================================================

    imageContainer =
        new QWidget(this);

    auto* imageLayout =
        new QVBoxLayout(
            imageContainer
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


    // =========================================================
    // КНОПКА СОХРАНИТЬ
    // =========================================================

    saveButton =
        new QPushButton(
            "Сохранить",
            imageContainer
        );

    saveButton->setFixedSize(
        180,
        50
    );

    saveButton->setCursor(
        Qt::PointingHandCursor
    );

    saveButton->hide();


    imageLayout->addWidget(
        saveButton,
        0,
        Qt::AlignCenter
    );


    connect(
        saveButton,
        &QPushButton::clicked,
        this,
        &MainWindow::saveAllImages
    );


    // =========================================================
    // PROGRESS BAR
    // =========================================================

    saveProgressBar =
        new QProgressBar(
            imageContainer
        );

    saveProgressBar->setFixedWidth(
        400
    );

    saveProgressBar->setFixedHeight(
        20
    );

    saveProgressBar->setRange(
        0,
        100
    );

    saveProgressBar->setValue(
        0
    );

    saveProgressBar->setTextVisible(
        true
    );

    saveProgressBar->hide();


    imageLayout->addWidget(
        saveProgressBar,
        0,
        Qt::AlignCenter
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
        "<a href=\"#\">← Назад (Esc / Beckspace)</a>"
    );

    markLabel->setText(
        "<a href=\"#\">Пометить (Space)</a>"
    );

    forwardLabel->setText(
        "<a href=\"#\">Вперёд → (Enter)</a>"
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

    connect(
        markLabel,
        &QLabel::linkActivated,
        this,
        &MainWindow::markCurrentPhoto
    );


    footerLayout->addWidget(
        backLabel
    );

    footerLayout->addStretch();

    footerLayout->addWidget(
        markLabel
    );

    footerLayout->addStretch();

    footerLayout->addWidget(
        forwardLabel
    );


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
    const QString suffix =
        QFileInfo(filePath)
        .suffix()
        .toLower();

    // Форматы, которые умеет Qt
    const QList<QByteArray> supportedFormats =
        QImageReader::supportedImageFormats();

    for (const QByteArray& format : supportedFormats)
    {
        if (QString::fromLatin1(format).toLower() == suffix)
            return true;
    }

    // Форматы, которые мы читаем сами
    static const QStringList customFormats =
    {
        "heic",
        "heif"
    };

    return customFormats.contains(suffix);
}

void MainWindow::showCurrentImage()
{
    saveButton->hide();
    saveProgressBar->hide();

    backLabel->show();
    markLabel->show();
    forwardLabel->show();

    updateStageHint();

    if (currentStage == Stage::Save)
    {
        cropCanvas->hide();

        saveButton->show();
        setFocus();

        saveProgressBar->hide();
        saveProgressBar->setValue(0);

        backLabel->show();
        markLabel->hide();
        forwardLabel->show();

        cropCanvas->setFocus();

        imageContainer->setStyleSheet("");

        photoCounterLabel->setText(
            "Быстрое кадрирование"
        );

        setWindowTitle(
            "Быстрое кадрирование"
        );

        return;
    }

    cropCanvas->show();
    saveButton->hide();
    saveProgressBar->hide();

    backLabel->show();
    markLabel->show();
    forwardLabel->show();

    forwardLabel->setText(
        "<a href=\"#\">Вперёд → (Enter)</a>"
    );

    if (currentImageIndex < 0 ||
        currentImageIndex >= images.size())
    {
        imageContainer->setStyleSheet("");

        cropCanvas->show();
        cropCanvas->setImage(QImage());

        photoCounterLabel->setText(
            "Быстрое кадрирование"
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

    imageContainer->setStyleSheet(
        "background-color: #808080;"
    );

    cropCanvas->show();

    if (currentStage == Stage::Light &&
        !item.croppedImage().isNull())
    {
        cropCanvas->setDisplayImage(
            item.croppedImage(),
            true
        );
    }
    else
    {
        cropCanvas->setDisplayImage(
            image,
            currentStage == Stage::Light
        );
    }

    // Берём последний размер, который ввёл пользователь.
    double width = lastCropWidth;
    double height = lastCropHeight;


    // Если это первое фото и пользователь ещё ничего не вводил,
    // берём стандартный размер фото.
    if (width <= 0 || height <= 0)
    {
        width = item.cropWidthCm();
        height = item.cropHeightCm();
    }


    // ориентация фотографии
    const bool imageLandscape =
        item.image().width() >
        item.image().height();


    // ориентация сохранённой рамки
    const bool cropLandscape =
        width > height;


    // если ориентация рамки не совпадает с фото,
    // меняем стороны местами
    if (imageLandscape != cropLandscape)
    {
        std::swap(width, height);
    }

    item.setCropSize(width, height);

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
        QString("Быстрое кадрирование %1 из %2")
        .arg(currentImageIndex + 1)
        .arg(images.size())
    );

    loadNewPhotosLabel->show();

    setWindowTitle(
        QString("Быстрое кадрирование - фото %1 из %2")
        .arg(currentImageIndex + 1)
        .arg(images.size())
    );
}

void MainWindow::loadDroppedUrls(const QList<QUrl>& urls)
{
    QStringList imageFiles;
    QStringList unsupportedFiles;

    // Запоминаем, что именно пользователь перетащил.
    saveSourceType =
        SaveSourceType::MultipleFiles;

    sourceDirectory.clear();

    if (urls.size() == 1)
    {
        const QString path =
            urls.first().toLocalFile();

        QFileInfo info(path);

        if (info.isDir())
        {
            saveSourceType =
                SaveSourceType::Directory;

            sourceDirectory =
                info.absoluteFilePath();
        }
        else if (info.isFile())
        {
            saveSourceType =
                SaveSourceType::SingleFile;

            sourceDirectory =
                info.absolutePath();
        }
    }

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
            "Быстрое кадрирование",
            "Изображения не найдены."
        );

        return;
    }

    if (saveSourceType == SaveSourceType::MultipleFiles &&
        !imageFiles.isEmpty())
    {
        sourceDirectory =
            QFileInfo(imageFiles.first()).absolutePath();
    }

    if (urls.size() == 1)
    {
        const QString path =
            urls.first().toLocalFile();

        QFileInfo info(path);

        if (info.isDir())
        {
            saveSourceType =
                SaveSourceType::Directory;

            sourceDirectory =
                info.absoluteFilePath();

            saveHash.clear();
        }
        else
        {
            saveSourceType =
                SaveSourceType::SingleFile;

            sourceDirectory.clear();
            saveHash.clear();
        }
    }
    else
    {
        saveSourceType =
            SaveSourceType::MultipleFiles;

        sourceDirectory.clear();

        QStringList fileNames;

        for (const QString& filePath : imageFiles)
        {
            fileNames.append(
                QFileInfo(filePath).fileName()
            );
        }

        fileNames.sort();

        const QByteArray hashData =
            fileNames.join("|").toUtf8();

        saveHash =
            QString::fromLatin1(
                QCryptographicHash::hash(
                    hashData,
                    QCryptographicHash::Sha256
                ).toHex().left(12)
            );
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

    currentStage = Stage::Crop;

    // Возвращаем обычное состояние интерфейса
    saveButton->hide();
    saveProgressBar->hide();

    cropHintsWidget->hide();

    backLabel->show();
    markLabel->show();
    forwardLabel->show();

    cropCanvas->show();
    cropCanvas->clearImage();

    // Возвращаем стандартный фон
    imageContainer->setStyleSheet("");

    photoCounterLabel->setText(
        "Быстрое кадрирование"
    );

    loadNewPhotosLabel->hide();

    updateStageHint();

    setWindowTitle(
        "Быстрое кадрирование"
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

    QStringList imageFiles;

    for (const QString& filePath : files)
    {
        if (isImageFile(filePath))
            imageFiles.append(filePath);
    }

    if (imageFiles.isEmpty())
        return;

    if (imageFiles.size() == 1)
    {
        saveSourceType =
            SaveSourceType::SingleFile;

        sourceDirectory.clear();
        saveHash.clear();
    }
    else
    {
        saveSourceType =
            SaveSourceType::MultipleFiles;

        sourceDirectory.clear();

        QStringList fileNames;

        for (const QString& filePath : imageFiles)
        {
            fileNames.append(
                QFileInfo(filePath).fileName()
            );
        }

        fileNames.sort();

        const QByteArray hashData =
            fileNames.join("|").toUtf8();

        saveHash =
            QString::fromLatin1(
                QCryptographicHash::hash(
                    hashData,
                    QCryptographicHash::Sha256
                ).toHex().left(12)
            );
    }

    loadImageFiles(imageFiles);
}

void MainWindow::applyCropSize()
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

    if (currentImageIndex >= 0 &&
        currentImageIndex < images.size())
    {
        images[currentImageIndex].setCropSize(
            clampedWidth,
            clampedHeight
        );
    }
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

    lastCropWidth = width;
    lastCropHeight = height;

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

    if (currentStage == Stage::Save)
    {
        saveAllImages();
        return;
    }

    // =========================================================
    // Кадр → Свет
    // =========================================================

    if (currentStage == Stage::Crop)
    {
        saveCurrentCropState();

        ImageItem& item =
            images[currentImageIndex];

        // =====================================================
        // SHIFT+ENTER → FIT MODE
        // =====================================================

        if (cropCanvas->isFitImageMode())
        {
            const QImage fittedImage =
                cropCanvas->createFitCropImage();

            if (!fittedImage.isNull())
            {
                item.setCroppedImage(
                    fittedImage
                );
            }
        }
        else
        {
            // =================================================
            // Обычное кадрирование
            // =================================================

            item.createCroppedImage(
                item.cropRect()
            );
        }

        currentStage =
            Stage::Light;

        updateStageHint();

        cropCanvas->setDisplayImage(
            item.croppedImage(),
            true
        );

        cropCanvas->setLevelsStatus(
            item.levelsStatus()
        );

        // Передаём сохранённое состояние Levels.
        cropCanvas->setLevelsStatus(
            item.levelsStatus()
        );

        return;
    }

    // =========================================================
    // Свет → следующая фотография
    // =========================================================

    if (currentStage == Stage::Light)
    {
        ImageItem& item =
            images[currentImageIndex];

        // =====================================================
        // Финально применяем свет
        // к полноразмерному изображению.
        // =====================================================

        const QImage finalImage =
            cropCanvas->createFinalLightImage();

        if (!finalImage.isNull())
        {
            item.setCroppedImage(
                finalImage
            );
        }

        // Последняя фотография.
        if (currentImageIndex >= images.size() - 1)
        {
            currentStage = Stage::Save;
            updateStageHint();
            showCurrentImage();
            return;
        }

        ++currentImageIndex;

        currentStage =
            Stage::Crop;

        updateStageHint();

        showCurrentImage();

        return;
    }
}

void MainWindow::goBack()
{
    if (images.isEmpty())
        return;

    if (currentStage == Stage::Save)
    {
        currentStage = Stage::Light;

        currentImageIndex = images.size() - 1;

        showCurrentImage();

        return;
    }

    // =========================================================
    // Свет → Кадр этой же фотографии
    // =========================================================

    if (currentStage == Stage::Light)
    {
        currentStage =
            Stage::Crop;

        updateStageHint();

        showCurrentImage();

        return;
    }

    // =========================================================
    // Кадр → Свет предыдущей фотографии
    // =========================================================

    if (currentStage == Stage::Crop)
    {
        // Сначала проверяем рамку
        if (!cropCanvas->isDefaultCrop())
        {
            cropCanvas->resetCrop();
            return;
        }


        // Если рамка уже стандартная —
        // идём на предыдущую фотографию
        if (currentImageIndex <= 0)
            return;


        saveCurrentCropState();

        --currentImageIndex;

        ImageItem& item =
            images[currentImageIndex];

        item.createCroppedImage(
            item.cropRect()
        );

        currentStage =
            Stage::Light;

        updateStageHint();

        cropCanvas->setDisplayImage(
            item.croppedImage(),
            true
        );

        cropCanvas->setLevelsStatus(
            item.levelsStatus()
        );

        cropCanvas->setCurvesStatus(
            item.curvesStatus()
        );

        updateLightStatusWidgets();

        return;
    }
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

    curvesImageLabel->setPixmap(
        levelsPixmaps[levelsIndex]
    );

    levelsImageLabel->setPixmap(
        curvesPixmaps[curvesIndex]
    );
}

void MainWindow::updateStageHint()
{
    if (currentStage == Stage::Crop)
    {
        if (!images.isEmpty())
        {
            cropHintsWidget->show();
            stageTitleLabel->setText("Настройка рамки");
        }
        else
        {
            stageTitleLabel->setText(
                ""
            );
        }

        lightStatusWidget->hide();

        return;
    }

    if (currentStage == Stage::Light)
    {
        stageTitleLabel->setText(
            "Настройка света"
        );

        cropHintsWidget->hide();
        lightStatusWidget->show();

        updateLightStatusWidgets();

        return;
    }

    // Save
    stageTitleLabel->setText(
        "Обработка завершена"
    );

    cropHintsWidget->hide();
    lightStatusWidget->hide();
}

void MainWindow::handleLightKey(QKeyEvent* event)
{
    if (currentStage != Stage::Light)
        return;

    if (currentImageIndex < 0 ||
        currentImageIndex >= images.size())
    {
        return;
    }

    ImageItem& item =
        images[currentImageIndex];

    switch (event->key())
    {
    case Qt::Key_Left:

        item.setLevelsStatus(
            item.levelsStatus() - 1
        );

        cropCanvas->setLevelsStatus(
            item.levelsStatus()
        );

        updateLightStatusWidgets();
        event->accept();
        return;

    case Qt::Key_Right:

        item.setLevelsStatus(
            item.levelsStatus() + 1
        );

        cropCanvas->setLevelsStatus(
            item.levelsStatus()
        );

        updateLightStatusWidgets();
        event->accept();
        return;

    case Qt::Key_Up:

        item.setCurvesStatus(
            item.curvesStatus() + 1
        );

        cropCanvas->setCurvesStatus(
            item.curvesStatus()
        );

        updateLightStatusWidgets();
        event->accept();
        return;

    case Qt::Key_Down:

        item.setCurvesStatus(
            item.curvesStatus() - 1
        );

        cropCanvas->setCurvesStatus(
            item.curvesStatus()
        );

        updateLightStatusWidgets();
        event->accept();
        return;
    default:
        return;
    }
}

void MainWindow::saveAllImages()
{
    if (images.isEmpty())
        return;

    // Есть ли хотя бы одна фотография,
    // которую действительно нужно сохранить.
    bool hasImagesToSave = false;

    for (const ImageItem& item : images)
    {
        if (!item.isMarked())
        {
            hasImagesToSave = true;
            break;
        }
    }

    if (!hasImagesToSave)
    {
        QMessageBox::information(
            this,
            "Сохранение",
            "Фотографий для сохранения нет."
        );

        qApp->quit();
        return;
    }

    // ---------------------------------------------------------
    // Определяем корневую папку сохранения
    // ---------------------------------------------------------

    if (saveSourceType != SaveSourceType::SingleFile)
    {
        saveDirectoryPath =
            makeUniquePath(
                getSaveDirectory()
            );

        if (!QDir().mkpath(saveDirectoryPath))
        {
            QMessageBox::critical(
                this,
                "Ошибка сохранения",
                "Не удалось создать папку:\n"
                + saveDirectoryPath
            );

            return;
        }
    }

    // ---------------------------------------------------------
    // Создаём папку Помеченные
    // ---------------------------------------------------------

    QString markedDirectory;

    bool hasMarkedImages = false;

    for (const ImageItem& item : images)
    {
        if (item.isMarked())
        {
            hasMarkedImages = true;
            break;
        }
    }

    if (hasMarkedImages)
    {
        markedDirectory =
            saveDirectoryPath + "/Помеченные";

        if (!QDir().mkpath(markedDirectory))
        {
            QMessageBox::critical(
                this,
                "Ошибка сохранения",
                "Не удалось создать папку:\n"
                + markedDirectory
            );

            return;
        }

        setFolderIcon(markedDirectory);
    }

    // ---------------------------------------------------------
    // Проверяем обработанные фотографии
    // ---------------------------------------------------------

    for (const ImageItem& item : images)
    {
        if (!item.isMarked() &&
            item.croppedImage().isNull())
        {
            QMessageBox::warning(
                this,
                "Ошибка",
                "Не все фотографии обработаны."
            );

            return;
        }
    }

    // ---------------------------------------------------------
    // ProgressBar
    // ---------------------------------------------------------

    saveButton->hide();

    saveProgressBar->show();
    saveProgressBar->setValue(0);

    QApplication::processEvents();

    const int filesToSave =
        images.size();

    int filesSaved = 0;

    // ---------------------------------------------------------
    // Сохранение
    // ---------------------------------------------------------

    for (const ImageItem& item : images)
    {
        // -----------------------------------------------------
        // Помеченная фотография
        // -----------------------------------------------------

        if (item.isMarked())
        {
            const QString fileName =
                QFileInfo(
                    item.filePath()
                ).fileName();

            const QString markedPath =
                markedDirectory
                + "/"
                + fileName;

            const QString uniqueMarkedPath =
                makeUniquePath(markedPath);

            if (!item.originalImage().save(
                uniqueMarkedPath))
            {
                QMessageBox::critical(
                    this,
                    "Ошибка сохранения",
                    "Не удалось сохранить:\n"
                    + item.filePath()
                );

                return;
            }
        }

        // -----------------------------------------------------
        // Обычная фотография
        // -----------------------------------------------------

        else
        {
            if (!saveImage(item))
            {
                QMessageBox::critical(
                    this,
                    "Ошибка сохранения",
                    "Не удалось сохранить:\n"
                    + item.filePath()
                );

                return;
            }
        }

        ++filesSaved;

        saveProgressBar->setValue(
            filesSaved * 100 / filesToSave
        );

        QApplication::processEvents();
    }

    // ---------------------------------------------------------
    // Сообщение об успешном сохранении
    // ---------------------------------------------------------

    QMessageBox messageBox(
        QMessageBox::Information,
        "Сохранение завершено",
        "Фотографии успешно сохранены.",
        QMessageBox::Ok,
        this
    );

    messageBox.setButtonText(
        QMessageBox::Ok,
        "ОК"
    );

    messageBox.exec();

    // ---------------------------------------------------------
    // Открываем папку
    // ---------------------------------------------------------

    if (saveSourceType == SaveSourceType::MultipleFiles ||
        saveSourceType == SaveSourceType::Directory)
    {
        QDesktopServices::openUrl(
            QUrl::fromLocalFile(
                saveDirectoryPath
            )
        );
    }

    // ---------------------------------------------------------
    // Показываем рабочий стол
    // ---------------------------------------------------------

    showDesktop();

    // ---------------------------------------------------------
    // Закрываем приложение
    // ---------------------------------------------------------

    qApp->quit();
}

QString MainWindow::getSaveDirectory() const
{
    const QString desktop =
        getDesktopPath();

    if (saveSourceType == SaveSourceType::Directory)
    {
        QFileInfo sourceInfo(sourceDirectory);

        return desktop
            + "/КАДР_"
            + sourceInfo.fileName();
    }

    if (saveSourceType == SaveSourceType::MultipleFiles)
    {
        return desktop
            + "/КАДР_ФОТКИ_"
            + saveHash;
    }

    return QString();
}

QString MainWindow::getSaveFilePath(
    const ImageItem& item
) const
{
    QFileInfo originalInfo(
        item.filePath()
    );


    QString extension = "jpg";

    if (item.croppedImage().hasAlphaChannel())
        extension = "png";


    // =========================================================
    // Один файл
    // =========================================================

    if (saveSourceType == SaveSourceType::SingleFile)
    {
        return makeUniquePath(
            getDesktopPath()
            + "/КАДР_"
            + originalInfo.completeBaseName()
            + "."
            + extension
        );
    }


    // =========================================================
    // Несколько файлов
    // =========================================================

    if (saveSourceType == SaveSourceType::MultipleFiles)
    {
        return saveDirectoryPath
            + "/"
            + originalInfo.completeBaseName()
            + "."
            + extension;
    }


    // =========================================================
    // Папка с подпапками
    // =========================================================

    QFileInfo sourceInfo(
        sourceDirectory
    );


    QDir sourceDir(
        sourceInfo.absoluteFilePath()
    );


    const QString relativePath =
        sourceDir.relativeFilePath(
            originalInfo.absoluteFilePath()
        );


    const QStringList parts =
        relativePath.split(
            QRegularExpression("[/\\\\]"),
            Qt::SkipEmptyParts
        );


    QString result =
        saveDirectoryPath;


    // Все элементы кроме последнего —
    // внутренние папки
    for (int i = 0; i < parts.size() - 1; ++i)
    {
        result +=
            "/КАДР_"
            + parts[i];
    }


    // Последний элемент — фотография.
    // Меняем только расширение.
    if (!parts.isEmpty())
    {
        QFileInfo fileInfo(
            parts.last()
        );


        result +=
            "/"
            + fileInfo.completeBaseName()
            + "."
            + extension;
    }


    return result;
}

bool MainWindow::saveImage(const ImageItem& item)
{
    const QString savePath =
        getSaveFilePath(item);


    QFileInfo fileInfo(savePath);


    QDir saveDir;

    if (!saveDir.mkpath(
        fileInfo.absolutePath()))
    {
        return false;
    }

    setFolderIcon(
        fileInfo.absolutePath()
    );


    QImage image =
        item.croppedImage();


    if (image.isNull())
        return false;


    QString format =
        fileInfo.suffix()
                .toLower();


    if (format == "jpg")
    {
        image =
            image.convertToFormat(
                QImage::Format_RGB32
            );
    }
    else if (format == "png")
    {
        image =
            image.convertToFormat(
                QImage::Format_ARGB32
            );
    }


    return image.save(
        savePath,
        format.toUpper()
              .toUtf8()
              .constData(),
        95
    );
}

QString MainWindow::getDesktopPath() const
{
    return QStandardPaths::writableLocation(
        QStandardPaths::DesktopLocation
    );
}

QString MainWindow::makeUniquePath(const QString& path) const
{
    if (!QFileInfo::exists(path))
        return path;

    QFileInfo info(path);

    const QString directory =
        info.absolutePath();

    const QString baseName =
        info.completeBaseName();

    const QString suffix =
        info.suffix();

    int number = 2;

    while (true)
    {
        QString newName =
            baseName
            + " ("
            + QString::number(number)
            + ")";

        if (!suffix.isEmpty())
            newName += "." + suffix;

        const QString newPath =
            QDir(directory).filePath(newName);

        if (!QFileInfo::exists(newPath))
            return newPath;

        ++number;
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (currentStage == Stage::Save &&
        (event->key() == Qt::Key_Return ||
            event->key() == Qt::Key_Enter))
    {
        saveAllImages();
        return;
    }

    if ((currentStage == Stage::Crop ||
            currentStage == Stage::Light ||
            currentStage == Stage::Save) &&
        (event->key() == Qt::Key_Escape ||
            event->key() == Qt::Key_Backspace))
    {
        goBack();
        return;
    }

    if ((currentStage == Stage::Crop ||
            currentStage == Stage::Light) &&
        event->key() == Qt::Key_Space)
    {
        markCurrentPhoto();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::showDesktop()
{
    IShellDispatch* shell = nullptr;

    HRESULT result =
        CoCreateInstance(
            CLSID_Shell,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_IShellDispatch,
            reinterpret_cast<void**>(&shell)
        );

    if (SUCCEEDED(result))
    {
        shell->MinimizeAll();
        shell->Release();
    }
}

void MainWindow::markCurrentPhoto()
{
    if (currentStage != Stage::Crop &&
        currentStage != Stage::Light)
    {
        return;
    }

    if (currentImageIndex < 0 ||
        currentImageIndex >= images.size())
    {
        return;
    }

    const QMessageBox::StandardButton result =
        QMessageBox::question(
            this,
            "Пометить фотографию",
            "Фотография будет пропущена и сохранится "
            "в папку \"Помеченные\" для дальнейшей обработки, "
            "например в Photoshop.\n\n"
            "Вы уверены, что хотите пропустить "
            "и пометить фотографию?",
            QMessageBox::Ok | QMessageBox::Cancel,
            QMessageBox::Cancel
        );

    if (result != QMessageBox::Ok)
        return;

    images[currentImageIndex].setMarked(true);

    // Переходим сразу к следующей фотографии.
    if (currentImageIndex < images.size() - 1)
    {
        ++currentImageIndex;

        currentStage = Stage::Crop;

        showCurrentImage();

        return;
    }

    // Последняя фотография была помечена.
    currentStage = Stage::Save;

    showCurrentImage();
}

void MainWindow::setFolderIcon(const QString& folderPath)
{
    QString desktopIniPath =
        QDir(folderPath).filePath("desktop.ini");

    QString exePath =
        QCoreApplication::applicationFilePath();

    QFile file(desktopIniPath);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);

        out << "[.ShellClassInfo]\n";
        out << "IconResource="
            << QDir::toNativeSeparators(exePath)
            << ",1\n";

        file.close();
    }

    SetFileAttributesW(
        reinterpret_cast<LPCWSTR>(
            QDir::toNativeSeparators(desktopIniPath).utf16()
        ),
        FILE_ATTRIBUTE_HIDDEN |
        FILE_ATTRIBUTE_SYSTEM
    );

    SetFileAttributesW(
        reinterpret_cast<LPCWSTR>(
            QDir::toNativeSeparators(folderPath).utf16()
        ),
        FILE_ATTRIBUTE_SYSTEM
    );
}

void MainWindow::focusImage()
{
    if (!images.isEmpty() &&
        cropCanvas->isVisible())
    {
        cropCanvas->setFocus();
    }
    else
    {
        clearFocus();
    }
}

void MainWindow::normalizeSizeInput(QLineEdit* edit)
{
    QString text = edit->text();

    QString result;
    bool hasSeparator = false;

    for (QChar ch : text)
    {
        if (ch.isDigit())
        {
            result += ch;
        }
        else
        {
            // любой символ превращаем в точку
            if (!hasSeparator)
            {
                result += '.';
                hasSeparator = true;
            }
        }
    }

    // если точка стоит первой — добавляем 0
    if (result.startsWith('.'))
    {
        result = "0" + result;
    }

    if (text != result)
    {
        int cursorPosition = edit->cursorPosition();

        edit->blockSignals(true);

        edit->setText(result);

        edit->setCursorPosition(
            qMin(cursorPosition, result.length())
        );

        edit->blockSignals(false);
    }
}
