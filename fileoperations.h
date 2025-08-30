#ifndef FILEOPERATIONS_H
#define FILEOPERATIONS_H

#include <QString>
#include <QStringList>

class FileOperations
{
public:
    FileOperations();

    QStringList scanFolders(const QString &mainFolder, bool recursive = false);
    QStringList getMediaFiles(const QString &folderPath, const QStringList &extensions);
    bool deleteFile(const QString &filePath);
    bool deleteFolder(const QString &folderPath);
    bool openFile(const QString &filePath);

private:
    void scanFoldersRecursive(const QString &folderPath, QStringList &folders);
};

#endif // FILEOPERATIONS_H
