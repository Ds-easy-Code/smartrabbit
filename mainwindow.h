#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QMap>
#include "configmanager.h"
#include "fileoperations.h"
// Remove: #include "mediadisplay.h" - we don't need it anymore

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override; // Add this

private slots:
    void on_browse_btn_clicked();
    void on_scan_btn_clicked();
    void on_prev_folder_btn_clicked();
    void on_next_folder_btn_clicked();
    void on_delete_folder_btn_clicked();
    void on_prev_media_btn_clicked();
    void on_next_media_btn_clicked();
    void on_play_btn_clicked();
    void on_delete_media_btn_clicked();

private:
    Ui::MainWindow *ui;
    ConfigManager configManager;
    FileOperations fileOperations;

    QString mainFolder;
    QStringList folders;
    int currentFolderIndex;
    QStringList mediaFiles;
    int currentMediaIndex;
    QStringList supportedExtensions;

    void updateFolderDisplay();
    void updateMediaDisplay();
    void updateButtonStates();
    void scanFolders();
    void showMessage(const QString &text, bool critical = false);

    // Keyboard event handling
    void keyPressEvent(QKeyEvent *event) override;
};
#endif // MAINWINDOW_H
