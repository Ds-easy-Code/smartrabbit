#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDate>
#include <QCheckBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Load configuration
    configManager.load();
    mainFolder = configManager.getMainFolder();
    supportedExtensions = configManager.getSupportedExtensions();

    // Initialize state
    currentFolderIndex = 0;
    currentMediaIndex = 0;

    // Setup UI
    ui->folder_entry->setText(mainFolder.isEmpty() ? "No folder selected" : mainFolder);
    ui->recursive_cb->setChecked(configManager.getRecursive());
    ui->skip_confirm_cb->setChecked(configManager.getSkipDeleteConfirmation());

    // Connect checkbox signals using checkStateChanged (not deprecated)
    connect(ui->recursive_cb, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState state) {
        configManager.setRecursive(state == Qt::Checked);
        configManager.save();
    });

    connect(ui->skip_confirm_cb, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState state) {
        configManager.setSkipDeleteConfirmation(state == Qt::Checked);
        configManager.save();
    });

    // Since ui->media_display is a QLabel, we can't connect MediaDisplay signals to it
    // We'll handle the click event differently - remove this connection:
    // connect(ui->media_display, &MediaDisplay::clicked, this, &MainWindow::on_play_btn_clicked);

    // Instead, we'll use an event filter or handle mouse events on the QLabel
    ui->media_display->installEventFilter(this);
    ui->media_display->setCursor(Qt::PointingHandCursor);

    // Initial button states
    updateButtonStates();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Add event filter to handle clicks on the media display
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->media_display && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            // Check if we're currently displaying a video
            QString currentFile = mediaFiles.value(currentMediaIndex);
            if (!currentFile.isEmpty()) {
                QString ext = QFileInfo(currentFile).suffix().toLower();
                bool isVideo = configManager.getVideoExtensions().contains("." + ext);
                if (isVideo) {
                    on_play_btn_clicked();
                    return true;
                }
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::on_browse_btn_clicked()
{
    QString folder = QFileDialog::getExistingDirectory(this, "Select Media Folder");
    if (!folder.isEmpty()) {
        mainFolder = folder;
        ui->folder_entry->setText(folder);
        configManager.setMainFolder(folder);
        configManager.save();
    }
}

void MainWindow::on_scan_btn_clicked()
{
    scanFolders();
}

void MainWindow::scanFolders()
{
    if (mainFolder.isEmpty()) {
        showMessage("Please select a folder first", true);
        return;
    }

    ui->status->setText("Scanning folders...");

    bool recursive = ui->recursive_cb->isChecked();
    folders = fileOperations.scanFolders(mainFolder, recursive);
    currentFolderIndex = 0;

    qDebug() << "Folders found:" << folders;

    if (!folders.isEmpty()) {
        ui->status->setText(QString("Found %1 folders").arg(folders.size()));
        updateFolderDisplay();
    } else {
        ui->status->setText("No folders found");
        ui->folder_info->setText("No folders found");
        updateButtonStates();
    }
}

void MainWindow::updateFolderDisplay()
{
    if (folders.isEmpty()) {
        ui->folder_info->setText("No folders found");
        mediaFiles.clear();
        currentMediaIndex = 0;
        updateMediaDisplay();
        updateButtonStates();
        return;
    }

    QString currentFolder = folders[currentFolderIndex];
    QString folderName = QFileInfo(currentFolder).fileName();
    ui->folder_info->setText(QString("%1 (%2/%3)").arg(folderName).arg(currentFolderIndex + 1).arg(folders.size()));

    qDebug() << "Loading media files from folder:" << currentFolder;

    // Load media files
    mediaFiles = fileOperations.getMediaFiles(currentFolder, supportedExtensions);

    qDebug() << "Media files found:" << mediaFiles;

    currentMediaIndex = 0;
    updateMediaDisplay();
    updateButtonStates();
}

void MainWindow::updateMediaDisplay()
{
    if (mediaFiles.isEmpty()) {
        ui->media_info->setText("No media files");
        ui->media_display->setText("No media selected");
        ui->media_display->setPixmap(QPixmap());
        ui->play_btn->setEnabled(false);
        ui->media_display->setCursor(Qt::ArrowCursor);
        qDebug() << "No media files to display";
        return;
    }

    QString currentFile = mediaFiles[currentMediaIndex];
    QString currentFolder = folders[currentFolderIndex];
    QString mediaPath = currentFolder + "/" + currentFile;

    qDebug() << "Displaying media file:" << mediaPath;

    // Check if video
    QString ext = QFileInfo(currentFile).suffix().toLower();
    bool isVideo = configManager.getVideoExtensions().contains("." + ext);

    qDebug() << "File extension:" << ext << "Is video:" << isVideo;

    // Load media
    if (isVideo) {
        // For video, show a placeholder or filename
        ui->media_display->setText("Video: " + currentFile + "\n\nClick to play");
        ui->media_display->setCursor(Qt::PointingHandCursor);
    } else {
        // For images, load and display the image
        QPixmap pixmap(mediaPath);
        if (!pixmap.isNull()) {
            QPixmap scaled = pixmap.scaled(ui->media_display->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            ui->media_display->setPixmap(scaled);
            ui->media_display->setText("");
            qDebug() << "Image loaded successfully";
        } else {
            ui->media_display->setText("Failed to load image: " + currentFile);
            qDebug() << "Failed to load image:" << mediaPath;
        }
        ui->media_display->setCursor(Qt::ArrowCursor);
    }

    ui->media_info->setText(QString("%1 (%2/%3)").arg(currentFile).arg(currentMediaIndex + 1).arg(mediaFiles.size()));
    ui->play_btn->setEnabled(isVideo);
    updateButtonStates();
}

void MainWindow::on_prev_folder_btn_clicked()
{
    if (currentFolderIndex > 0) {
        currentFolderIndex--;
        updateFolderDisplay();
    }
}

void MainWindow::on_next_folder_btn_clicked()
{
    if (currentFolderIndex < folders.size() - 1) {
        currentFolderIndex++;
        updateFolderDisplay();
    }
}

void MainWindow::on_delete_folder_btn_clicked()
{
    if (folders.isEmpty()) return;

    QString folderToDelete = folders[currentFolderIndex];
    QString folderName = QFileInfo(folderToDelete).fileName();

    if (!ui->skip_confirm_cb->isChecked()) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete Folder",
                                                                  QString("Delete folder \"%1\" and all contents?").arg(folderName),
                                                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (reply != QMessageBox::Yes) return;
    }

    if (fileOperations.deleteFolder(folderToDelete)) {
        folders.removeAt(currentFolderIndex);
        if (currentFolderIndex >= folders.size()) {
            currentFolderIndex = qMax(0, folders.size() - 1);
        }
        updateFolderDisplay();
        ui->status->setText(QString("Deleted folder: %1").arg(folderName));
    } else {
        showMessage(QString("Failed to delete folder: %1").arg(folderName), true);
    }
}

void MainWindow::on_prev_media_btn_clicked()
{
    if (currentMediaIndex > 0) {
        currentMediaIndex--;
        updateMediaDisplay();
    }
}

void MainWindow::on_next_media_btn_clicked()
{
    if (currentMediaIndex < mediaFiles.size() - 1) {
        currentMediaIndex++;
        updateMediaDisplay();
    }
}

void MainWindow::on_play_btn_clicked()
{
    if (mediaFiles.isEmpty()) return;

    QString currentFile = mediaFiles[currentMediaIndex];
    QString currentFolder = folders[currentFolderIndex];
    QString filePath = currentFolder + "/" + currentFile;

    if (!fileOperations.openFile(filePath)) {
        showMessage("Failed to play video", true);
    }
}

void MainWindow::on_delete_media_btn_clicked()
{
    if (mediaFiles.isEmpty()) return;

    QString currentFile = mediaFiles[currentMediaIndex];
    QString currentFolder = folders[currentFolderIndex];
    QString filePath = currentFolder + "/" + currentFile;

    if (!ui->skip_confirm_cb->isChecked()) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete Media",
                                                                  QString("Delete \"%1\"?").arg(currentFile),
                                                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (reply != QMessageBox::Yes) return;
    }

    if (fileOperations.deleteFile(filePath)) {
        mediaFiles.removeAt(currentMediaIndex);
        if (currentMediaIndex >= mediaFiles.size()) {
            currentMediaIndex = qMax(0, mediaFiles.size() - 1);
        }
        updateMediaDisplay();
        ui->status->setText(QString("Deleted: %1").arg(currentFile));
    } else {
        showMessage(QString("Failed to delete: %1").arg(currentFile), true);
    }
}

void MainWindow::showMessage(const QString &text, bool critical)
{
    QMessageBox msg;
    msg.setIcon(critical ? QMessageBox::Critical : QMessageBox::Information);
    msg.setText(text);
    msg.setWindowTitle(critical ? "Error" : "Info");
    msg.exec();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    bool ctrl = event->modifiers() & Qt::ControlModifier;

    // Folder navigation with Ctrl
    if (ctrl) {
        switch (event->key()) {
        case Qt::Key_Left:
            on_prev_folder_btn_clicked();
            break;
        case Qt::Key_Right:
            on_next_folder_btn_clicked();
            break;
        case Qt::Key_Delete:
            on_delete_folder_btn_clicked();
            break;
        default:
            QMainWindow::keyPressEvent(event);
        }
    } else {
        // Media navigation
        switch (event->key()) {
        case Qt::Key_Left:
            on_prev_media_btn_clicked();
            break;
        case Qt::Key_Right:
            on_next_media_btn_clicked();
            break;
        case Qt::Key_Delete:
            on_delete_media_btn_clicked();
            break;
        case Qt::Key_Space:
            on_play_btn_clicked();
            break;
        default:
            QMainWindow::keyPressEvent(event);
        }
    }
}

void MainWindow::updateButtonStates()
{
    bool hasFolders = !folders.isEmpty();
    bool hasMedia = !mediaFiles.isEmpty();

    // Folder buttons
    ui->prev_folder_btn->setEnabled(hasFolders && currentFolderIndex > 0);
    ui->next_folder_btn->setEnabled(hasFolders && currentFolderIndex < folders.size() - 1);
    ui->delete_folder_btn->setEnabled(hasFolders);

    // Media buttons
    ui->prev_media_btn->setEnabled(hasMedia && currentMediaIndex > 0);
    ui->next_media_btn->setEnabled(hasMedia && currentMediaIndex < mediaFiles.size() - 1);
    ui->delete_media_btn->setEnabled(hasMedia);
    ui->play_btn->setEnabled(hasMedia && !mediaFiles.isEmpty());
}
