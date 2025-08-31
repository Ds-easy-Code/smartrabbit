#include "fileoperations.h"
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug> // Add for debugging

FileOperations::FileOperations()
{
}

QStringList FileOperations::scanFolders(const QString &mainFolder, bool recursive)
{
    QStringList folders;

    // Always include the main folder itself when recursive is enabled
    if (recursive) {
        folders.append(mainFolder); // Add the main folder
    }

    if (!recursive) {
        QDir dir(mainFolder);
        QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &entry : entries) {
            folders.append(dir.filePath(entry));
        }
    } else {
        scanFoldersRecursive(mainFolder, folders);
    }

    return folders;
}

void FileOperations::scanFoldersRecursive(const QString &folderPath, QStringList &folders)
{
    QDir dir(folderPath);
    QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &entry : entries) {
        QString fullPath = dir.filePath(entry);
        folders.append(fullPath);
        scanFoldersRecursive(fullPath, folders);
    }
}

QStringList FileOperations::getMediaFiles(const QString &folderPath, const QStringList &extensions)
{
    QDir dir(folderPath);
    QStringList files = dir.entryList(QDir::Files);
    QStringList mediaFiles;

    qDebug() << "Scanning folder:" << folderPath;
    qDebug() << "Files found:" << files;
    qDebug() << "Looking for extensions:" << extensions;

    for (const QString &file : files) {
        QString ext = "." + QFileInfo(file).suffix().toLower();
        if (extensions.contains(ext)) {
            qDebug() << "Found media file:" << file << "with extension:" << ext;
            mediaFiles.append(file);
        }
    }

    qDebug() << "Media files found:" << mediaFiles;
    return mediaFiles;
}

bool FileOperations::deleteFile(const QString &filePath)
{
    return QFile::remove(filePath);
}

bool FileOperations::deleteFolder(const QString &folderPath)
{
    QDir dir(folderPath);
    return dir.removeRecursively();
}

bool FileOperations::openFile(const QString &filePath)
{
    return QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
}
