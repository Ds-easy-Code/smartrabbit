#ifndef MEDIADISPLAY_H
#define MEDIADISPLAY_H

#include <QLabel>
#include <QFile>
#include <QMouseEvent>

class MediaDisplay : public QLabel
{
    Q_OBJECT

public:
    explicit MediaDisplay(QWidget *parent = nullptr);
    void setMedia(const QString &path, bool isVideo);

signals:
    void clicked();  // Make sure this is declared as a signal

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    bool isVideo = false;
    QString currentPath;
    QPixmap currentPixmap;
    void updatePixmap();
};

#endif // MEDIADISPLAY_H
