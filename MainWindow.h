#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QDialog>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFontComboBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QPushButton>
#include <QtGui/QKeyEvent>
#include <QtGui/QResizeEvent>
#include <QShortcut>
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
    void openFile();
    void saveAs();
    void exportFile();
    void showPreferences();
    void showEditorContextMenu(const QPoint &pos);
    void toggleDistractionFreeMode();
    void handleTopHover(bool entered);
    void handleBottomHover(bool entered);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupToolbar();
    void setupMenuBar();
    void updateTheme();
    void saveCurrentFile();
    void setupDistractionFreeMode();
    void enterDistractionFreeMode();
    void exitDistractionFreeMode();
    void updateEditorMargins();
    void updateHoverZones();
    void updateOverlayGeometry();

    EditorWidget *m_editorWidget;
    FileTreeWidget *m_fileTreeWidget;
    WelcomeWidget *m_welcomeWidget;
    QString m_currentFile;
    QToolBar *m_formatToolBar;
    QAction *m_boldAction;
    QAction *m_italicAction;
    QAction *m_underlineAction;
    QAction *m_distractionFreeAction;
    QIcon m_distractionFreeIcon;

    bool m_isDistractionFree;
    QWidget *m_topHoverZone;
    QWidget *m_bottomHoverZone;
    int m_distractionFreeMarginChars;
    QMenuBar *m_menuBar; // Store menubar pointer for showing/hiding

    // Overlay widget for distraction-free mode
    QWidget *m_overlay;
    QVBoxLayout *m_overlayLayout;

    // Store widget states for restoration
    bool m_wasToolbarVisible;
    bool m_wasSidebarVisible;
    QWidget *m_menuBarParent;
    QWidget *m_toolbarParent;
};