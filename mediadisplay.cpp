#include "mediadisplay.h"
#include <QPixmap>
#include <QPainter>
#include <QColor>
#include <QMouseEvent>
#include <QResizeEvent>

MediaDisplay::MediaDisplay(QWidget *parent) : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
    setMinimumHeight(300);
    setStyleSheet(R"(
        QLabel {
            background-color: #f8f9fa;
            border: 1px solid #e0e0e0;
            border-radius: 4px;
            color: #666;
            font-size: 14px;
        }
    )");
}

void MediaDisplay::setMedia(const QString &path, bool video)
{
    isVideo = video;
    currentPath = path;

    if (path.isEmpty() || !QFile::exists(path)) {
        setText("No media selected");
        currentPixmap = QPixmap();
        return;
    }

    // Only reload if path changed
    if (currentPixmap.isNull() || currentPath != path) {
        currentPixmap.load(path);
        if (currentPixmap.isNull()) {
            setText("Failed to load media");
            return;
        }
    }

    updatePixmap();
}

void MediaDisplay::updatePixmap()
{
    if (currentPixmap.isNull()) {
        setText("No media to display");
        return;
    }

    // Scale to fit while maintaining aspect ratio
    QPixmap scaled = currentPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    setPixmap(scaled);
}

void MediaDisplay::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    if (!currentPixmap.isNull()) {
        updatePixmap();
    }
}

void MediaDisplay::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);

    if (isVideo && !pixmap().isNull()) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // Calculate center position
        QRect rect = this->rect();
        int size = qMin(rect.width(), rect.height()) / 6;
        size = qMax(size, 40);

        int center_x = rect.center().x() - size / 2;
        int center_y = rect.center().y() - size / 2;

        // Draw play button background
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 120));
        painter.drawEllipse(center_x, center_y, size, size);

        // Draw play triangle
        painter.setBrush(QColor(255, 255, 255, 200));
        int triangle_size = size / 3;
        int offset = 2;

        QPoint center = rect.center();
        QPolygon polygon;
        polygon << QPoint(center.x() - triangle_size/2 + offset, center.y() - triangle_size/2)
                << QPoint(center.x() - triangle_size/2 + offset, center.y() + triangle_size/2)
                << QPoint(center.x() + triangle_size/2 + offset, center.y());

        painter.drawPolygon(polygon);
    }
}

void MediaDisplay::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isVideo) {
        emit clicked();
    }
    QLabel::mousePressEvent(event);
}
