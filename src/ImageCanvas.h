#pragma once

#include <QWidget>
#include <QImage>

class ImageCanvas : public QWidget
{
public:
    explicit ImageCanvas(QWidget *parent = nullptr);

    void setImage(const QImage &image);
    void clearImage();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QImage image;
};