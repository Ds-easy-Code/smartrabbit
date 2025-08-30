#include "configmanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>

ConfigManager::ConfigManager()
{
    // Use application directory for config file
    configFile = QDir::currentPath() + "/media_organizer.json";
}

bool ConfigManager::load()
{
    QFile file(configFile);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        return false;
    }

    QJsonObject config = doc.object();
    mainFolder = config.value("main_folder").toString("");
    recursive = config.value("recursive").toBool(false);
    skipDeleteConfirmation = config.value("skip_delete_confirmation").toBool(false);

    return true;
}

bool ConfigManager::save()
{
    QJsonObject config;
    config.insert("main_folder", mainFolder);
    config.insert("recursive", recursive);
    config.insert("skip_delete_confirmation", skipDeleteConfirmation);

    QJsonDocument doc(config);
    QFile file(configFile);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(doc.toJson());
    file.close();
    return true;
}

QStringList ConfigManager::getSupportedExtensions() const
{
    QStringList extensions;
    extensions.append(imageExtensions);
    extensions.append(videoExtensions);
    return extensions;
}
