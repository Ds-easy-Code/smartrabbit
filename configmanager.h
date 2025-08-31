#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>
#include <QStringList>
#include <QMap>

class ConfigManager
{
public:
    ConfigManager();

    bool load();
    bool save();

    QString getMainFolder() const { return mainFolder; }
    void setMainFolder(const QString &folder) { mainFolder = folder; }

    bool getRecursive() const { return recursive; }
    void setRecursive(bool value) { recursive = value; }

    bool getSkipDeleteConfirmation() const { return skipDeleteConfirmation; }
    void setSkipDeleteConfirmation(bool value) { skipDeleteConfirmation = value; }

    QStringList getSupportedExtensions() const;
    QStringList getImageExtensions() const { return imageExtensions; }
    QStringList getVideoExtensions() const { return videoExtensions; }

private:
    QString configFile = "media_organizer.json";
    QString mainFolder;
    bool recursive = false;
    bool skipDeleteConfirmation = false;
    QStringList imageExtensions = {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp"};
    QStringList videoExtensions = {".mp4", ".avi", ".mov", ".mkv", ".wmv", ".flv", ".m4v", ".webm"};
};

#endif // CONFIGMANAGER_H
