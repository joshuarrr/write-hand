#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QSplitter>
#include "EditorWidget.h"
#include "FileTreeWidget.h"
#include "WelcomeWidget.h"
#include "ThemeManager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void onFileSelected(const QString &filePath);
    void onFileCreated(const QString &filePath);
    void onFileRenamed(const QString &oldPath, const QString &newPath);
    void onFileDeleted(const QString &filePath);
    void onContentChanged();
    void toggleSidebar();
    void setBold();
    void setItalic();
    void setUnderline();
    void onThemeChanged(bool isDarkMode);

private:
    void setupToolbar();
    void updateTheme();
    void saveCurrentFile();

    EditorWidget *m_editorWidget;
    FileTreeWidget *m_fileTreeWidget;
    WelcomeWidget *m_welcomeWidget;
    QSplitter *m_splitter;
    QString m_currentFile;
    QToolBar *m_formatToolBar;
};

#endif // MAINWINDOW_H