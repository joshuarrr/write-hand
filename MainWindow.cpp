#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <QtGui/QGuiApplication>
#include <QtGui/QStyleHints>
#include <QtWidgets/QStackedLayout>
#include <QtWidgets/QToolBar>
#include <QtCore/QDebug>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtSvg/QSvgRenderer>

// Test comment to verify watch script
// Another test comment to verify rebuild
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_editorWidget(new EditorWidget(this)), m_fileTreeWidget(new FileTreeWidget(this)), m_welcomeWidget(new WelcomeWidget(this)), m_formatToolBar(nullptr)
{
    // Create a container widget for the editor area
    QWidget *editorContainer = new QWidget(this);
    QStackedLayout *stackedLayout = new QStackedLayout(editorContainer);
    stackedLayout->addWidget(m_welcomeWidget); // Add welcome widget first
    stackedLayout->addWidget(m_editorWidget);  // Add editor widget second

    // Create a container for the file tree and editor
    QWidget *contentContainer = new QWidget(this);
    QHBoxLayout *contentLayout = new QHBoxLayout(contentContainer);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // Create a splitter
    QSplitter *splitter = new QSplitter(Qt::Horizontal, contentContainer);
    splitter->addWidget(m_fileTreeWidget);
    splitter->addWidget(editorContainer);

    // Style the splitter
    splitter->setStyleSheet(
        "QSplitter::handle { "
        "   background-color: #2D2D2D; "
        "   width: 1px; "
        "} "
        "QSplitter::handle:hover { "
        "   background-color: #3D3D3D; "
        "}");

    // Set initial sizes (40% for file tree, 60% for editor)
    QList<int> sizes;
    sizes << 400 << 600;
    splitter->setSizes(sizes);

    // Set minimum sizes to prevent columns from disappearing
    m_fileTreeWidget->setMinimumWidth(300);
    editorContainer->setMinimumWidth(400);

    // Add splitter to layout
    contentLayout->addWidget(splitter);

    // Set size policies
    m_fileTreeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    editorContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setCentralWidget(contentContainer);
    setupToolbar();

    // Connect signals
    connect(m_fileTreeWidget, &FileTreeWidget::fileSelected, this, &MainWindow::onFileSelected);
    connect(m_fileTreeWidget, &FileTreeWidget::fileCreated, this, &MainWindow::onFileCreated);
    connect(m_fileTreeWidget, &FileTreeWidget::fileRenamed, this, &MainWindow::onFileRenamed);
    connect(m_fileTreeWidget, &FileTreeWidget::fileDeleted, this, &MainWindow::onFileDeleted);
    connect(m_editorWidget, &EditorWidget::contentChanged, this, &MainWindow::onContentChanged);
    connect(m_welcomeWidget, &WelcomeWidget::newFileRequested, m_fileTreeWidget, &FileTreeWidget::createNewFile);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &MainWindow::onThemeChanged);

    // Check for existing files
    QString appPath = QDir::homePath() + "/Documents/WriteHand";
    QDir appDir(appPath);

    // Create the directory if it doesn't exist
    if (!appDir.exists())
    {
        appDir.mkpath(".");
    }

    QStringList filters;
    filters << "*.txt" << "*.md" << "*.rtf" << "*.html" << "*.markdown" << "*.text";
    QFileInfoList files = appDir.entryInfoList(filters, QDir::Files, QDir::Time); // Sort by time, newest first

    if (files.isEmpty())
    {
        // Show welcome widget if no files exist
        stackedLayout->setCurrentWidget(m_welcomeWidget);
        m_editorWidget->hide();
        m_welcomeWidget->show();
    }
    else
    {
        // Open the most recently modified file
        stackedLayout->setCurrentWidget(m_editorWidget);
        m_welcomeWidget->hide();
        m_editorWidget->show();
        onFileSelected(files.first().filePath());
    }

    setWindowTitle("WriteHand");
    resize(1024, 768);

    // Initial theme update
    updateTheme();
}

void MainWindow::updateTheme()
{
    auto &theme = ThemeManager::instance();
    setStyleSheet(QString("QMainWindow { background-color: %1; }").arg(theme.getColor("background")));

    if (m_formatToolBar)
    {
        m_formatToolBar->setStyleSheet(theme.getStyleSheet("toolbar"));
    }
}

void MainWindow::onThemeChanged(bool isDarkMode)
{
    Q_UNUSED(isDarkMode);
    updateTheme();
}

void MainWindow::setupToolbar()
{
    m_formatToolBar = addToolBar("Formatting");
    m_formatToolBar->setMovable(false);

    // Left section - File controls
    QAction *sidebarAction = new QAction(this);

    // Try different paths for sidebar icon
    QStringList sidebarPaths = {
        QCoreApplication::applicationDirPath() + "/../Resources/icons/sidebar.svg",
        QCoreApplication::applicationDirPath() + "/Contents/Resources/icons/sidebar.svg",
        ":/icons/sidebar.svg",
        "../icons/sidebar.svg", // Development path
        "icons/sidebar.svg"     // Direct path
    };

    QIcon sidebarIcon;
    for (const QString &path : sidebarPaths)
    {
        QFile file(path);
        if (file.exists())
        {
            // Load SVG content
            file.open(QIODevice::ReadOnly);
            QByteArray content = file.readAll();
            file.close();

            // Replace currentColor with #E0E0E0 in the SVG content
            QString svgContent = QString::fromUtf8(content);
            svgContent.replace("currentColor", "#E0E0E0");

            // Create colored versions for different states
            QIcon icon;
            QPixmap pixmap(24, 24);

            // Normal state
            pixmap.fill(Qt::transparent);
            QPainter painter(&pixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            QSvgRenderer renderer(svgContent.toUtf8());
            renderer.render(&painter);
            icon.addPixmap(pixmap, QIcon::Normal, QIcon::Off);

            // Hover/Selected state (brighter)
            pixmap.fill(Qt::transparent);
            svgContent.replace("#E0E0E0", "#FFFFFF");
            QSvgRenderer rendererHover(svgContent.toUtf8());
            rendererHover.render(&painter);
            icon.addPixmap(pixmap, QIcon::Selected, QIcon::Off);
            icon.addPixmap(pixmap, QIcon::Active, QIcon::Off);

            // Disabled state (darker)
            pixmap.fill(Qt::transparent);
            svgContent.replace("#FFFFFF", "#808080");
            QSvgRenderer rendererDisabled(svgContent.toUtf8());
            rendererDisabled.render(&painter);
            icon.addPixmap(pixmap, QIcon::Disabled, QIcon::Off);

            painter.end();

            if (!icon.isNull())
            {
                sidebarIcon = icon;
                qDebug() << "Loaded sidebar icon from:" << path;
                break;
            }
        }
    }

    // Use text icon as fallback only if no SVG is found
    if (sidebarIcon.isNull())
    {
        qDebug() << "Failed to load sidebar icon from paths:" << sidebarPaths;
        sidebarAction->setText("☰");
        QFont font = sidebarAction->font();
        font.setPointSize(14);
        sidebarAction->setFont(font);
    }
    else
    {
        sidebarAction->setIcon(sidebarIcon);
    }

    sidebarAction->setToolTip("Toggle Sidebar");
    sidebarAction->setCheckable(true);
    sidebarAction->setChecked(true);
    connect(sidebarAction, &QAction::triggered, this, &MainWindow::toggleSidebar);

    // Add the action and get its QToolButton
    m_formatToolBar->addAction(sidebarAction);
    if (QToolButton *sidebarButton = qobject_cast<QToolButton *>(m_formatToolBar->widgetForAction(sidebarAction)))
    {
        sidebarButton->setObjectName("sidebarButton");
    }

    // Set application icon
    QStringList appIconPaths = {
        QCoreApplication::applicationDirPath() + "/../Resources/icons/app.svg",
        QCoreApplication::applicationDirPath() + "/Contents/Resources/icons/app.svg",
        ":/icons/app.svg",
        "../icons/app.svg", // Development path
        "icons/app.svg"     // Direct path
    };

    QIcon appIcon;
    for (const QString &path : appIconPaths)
    {
        QFile file(path);
        if (file.exists())
        {
            appIcon = QIcon(path);
            if (!appIcon.isNull())
            {
                qDebug() << "Loaded app icon from:" << path;
                break;
            }
        }
    }

    if (!appIcon.isNull())
    {
        setWindowIcon(appIcon);
        qApp->setWindowIcon(appIcon);
    }
    else
    {
        qDebug() << "Failed to load app icon from paths:" << appIconPaths;
    }

    m_formatToolBar->addSeparator();

    // File operations - New Document
    QIcon newIcon = QIcon::fromTheme("document-new");
    QPixmap newPixmap(24, 24);
    newPixmap.fill(Qt::transparent);
    QPainter newPainter(&newPixmap);
    newPainter.setRenderHint(QPainter::Antialiasing);
    newPainter.setPen(QPen(QColor("#E0E0E0")));
    newPainter.setBrush(Qt::NoBrush);

    // Draw a simple document icon with plus
    newPainter.drawRect(6, 4, 12, 16);
    newPainter.drawLine(12, 8, 12, 16); // Vertical line of plus
    newPainter.drawLine(8, 12, 16, 12); // Horizontal line of plus
    newPainter.end();

    QIcon customNewIcon;
    customNewIcon.addPixmap(newPixmap, QIcon::Normal, QIcon::Off);

    // Create brighter version for hover
    QPixmap hoverPixmap = newPixmap;
    hoverPixmap.fill(Qt::transparent);
    QPainter hoverPainter(&hoverPixmap);
    hoverPainter.setRenderHint(QPainter::Antialiasing);
    hoverPainter.setPen(QPen(QColor("#FFFFFF")));
    hoverPainter.setBrush(Qt::NoBrush);
    hoverPainter.drawRect(6, 4, 12, 16);
    hoverPainter.drawLine(12, 8, 12, 16);
    hoverPainter.drawLine(8, 12, 16, 12);
    hoverPainter.end();
    customNewIcon.addPixmap(hoverPixmap, QIcon::Active, QIcon::Off);
    customNewIcon.addPixmap(hoverPixmap, QIcon::Selected, QIcon::Off);

    QAction *newAction = m_formatToolBar->addAction(customNewIcon, "New");
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, m_fileTreeWidget, &FileTreeWidget::createNewFile);

    // Add a stretcher to push formatting controls to the right
    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_formatToolBar->addWidget(spacer);

    // Right section - Text formatting
    m_formatToolBar->addSeparator();

    QAction *boldAction = m_formatToolBar->addAction(QIcon::fromTheme("format-text-bold"), "Bold");
    boldAction->setCheckable(true);
    boldAction->setShortcut(QKeySequence::Bold);
    connect(boldAction, &QAction::triggered, this, &MainWindow::setBold);

    QAction *italicAction = m_formatToolBar->addAction(QIcon::fromTheme("format-text-italic"), "Italic");
    italicAction->setCheckable(true);
    italicAction->setShortcut(QKeySequence::Italic);
    connect(italicAction, &QAction::triggered, this, &MainWindow::setItalic);

    QAction *underlineAction = m_formatToolBar->addAction(QIcon::fromTheme("format-text-underline"), "Underline");
    underlineAction->setCheckable(true);
    underlineAction->setShortcut(QKeySequence::Underline);
    connect(underlineAction, &QAction::triggered, this, &MainWindow::setUnderline);

    // Apply initial theme
    m_formatToolBar->setStyleSheet(ThemeManager::instance().getStyleSheet("toolbar"));
}

void MainWindow::onFileSelected(const QString &filePath)
{
    saveCurrentFile(); // Save current file before switching
    m_currentFile = filePath;

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        bool isRichText = filePath.endsWith(".rtf", Qt::CaseInsensitive);
        m_editorWidget->setContent(in.readAll(), isRichText);
        file.close();

        // Switch to editor widget
        QStackedLayout *stackedLayout = qobject_cast<QStackedLayout *>(m_editorWidget->parentWidget()->layout());
        if (stackedLayout)
        {
            stackedLayout->setCurrentWidget(m_editorWidget);
            m_welcomeWidget->hide();
            m_editorWidget->show();
        }

        setWindowTitle("WriteHand - " + QFileInfo(filePath).fileName());

        // Select the file in the tree
        m_fileTreeWidget->selectFile(filePath);
    }
}

void MainWindow::onFileCreated(const QString &filePath)
{
    saveCurrentFile();
    m_currentFile = filePath;
    m_editorWidget->clear();

    // Switch to editor widget
    QStackedLayout *stackedLayout = qobject_cast<QStackedLayout *>(m_editorWidget->parentWidget()->layout());
    if (stackedLayout)
    {
        stackedLayout->setCurrentWidget(m_editorWidget);
        m_welcomeWidget->hide();
        m_editorWidget->show();
    }

    setWindowTitle("WriteHand - " + QFileInfo(filePath).fileName());

    // Select the new file in the tree and ensure focus
    m_fileTreeWidget->selectFile(filePath);
    m_editorWidget->setFocus();
    m_editorWidget->editor()->setFocus(); // Explicitly focus the text editor
}

void MainWindow::onFileRenamed(const QString &oldPath, const QString &newPath)
{
    if (m_currentFile == oldPath)
    {
        m_currentFile = newPath;
        setWindowTitle("WriteHand - " + QFileInfo(newPath).fileName());
    }
}

void MainWindow::onFileDeleted(const QString &filePath)
{
    if (m_currentFile == filePath)
    {
        m_currentFile.clear();
        m_editorWidget->clear();

        // Check if this was the last file
        QString appPath = QDir::homePath() + "/Documents/WriteHand";
        QDir appDir(appPath);

        QStringList filters;
        filters << "*.txt" << "*.md" << "*.rtf" << "*.html" << "*.markdown" << "*.text";
        QFileInfoList files = appDir.entryInfoList(filters, QDir::Files, QDir::Time);

        if (files.isEmpty())
        {
            // Show welcome widget if no files exist
            QStackedLayout *stackedLayout = qobject_cast<QStackedLayout *>(m_editorWidget->parentWidget()->layout());
            if (stackedLayout)
            {
                stackedLayout->setCurrentWidget(m_welcomeWidget);
                m_editorWidget->hide();
                m_welcomeWidget->show();
            }
            setWindowTitle("WriteHand");
        }
        else
        {
            // Open the most recently modified file
            onFileSelected(files.first().filePath());
        }
    }
}

void MainWindow::onContentChanged()
{
    saveCurrentFile();
}

void MainWindow::saveCurrentFile()
{
    if (m_currentFile.isEmpty())
        return;

    QFile file(m_currentFile);
    if (!file.open(QIODevice::WriteOnly))
        return;

    QTextStream out(&file);
    bool isRichText = m_currentFile.endsWith(".rtf", Qt::CaseInsensitive);
    out << m_editorWidget->content(isRichText);
    file.close();
}

void MainWindow::toggleSidebar()
{
    m_fileTreeWidget->setVisible(!m_fileTreeWidget->isVisible());
}

void MainWindow::setBold()
{
    QTextCharFormat format;
    format.setFontWeight(m_editorWidget->editor()->fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
    m_editorWidget->editor()->textCursor().mergeCharFormat(format);
}

void MainWindow::setItalic()
{
    QTextCharFormat format;
    format.setFontItalic(!m_editorWidget->editor()->fontItalic());
    m_editorWidget->editor()->textCursor().mergeCharFormat(format);
}

void MainWindow::setUnderline()
{
    QTextCharFormat format;
    format.setFontUnderline(!m_editorWidget->editor()->fontUnderline());
    m_editorWidget->editor()->textCursor().mergeCharFormat(format);
}