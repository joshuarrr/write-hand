#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <QtGui/QGuiApplication>
#include <QtGui/QStyleHints>
#include <QtWidgets/QStackedLayout>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMenu>
#include <QtCore/QDebug>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtSvg/QSvgRenderer>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtPrintSupport/QPrinter>
#include <QtGui/QPainter>

// Test comment to verify watch script
// Another test comment to verify rebuild
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_editorWidget(new EditorWidget(this)), m_fileTreeWidget(new FileTreeWidget(this)), m_welcomeWidget(new WelcomeWidget(this)), m_formatToolBar(nullptr)
{
    setupMenuBar();

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

    // File operations
    QAction *newAction = m_formatToolBar->addAction(QIcon::fromTheme("document-new"), "New");
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

void MainWindow::setupMenuBar()
{
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    // File Menu
    QMenu *fileMenu = menuBar->addMenu("File");
    QAction *newAction = new QAction("New", this);
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, m_fileTreeWidget, &FileTreeWidget::createNewFile);
    fileMenu->addAction(newAction);

    QAction *openAction = new QAction("Open...", this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(openAction);

    fileMenu->addSeparator();

    QAction *saveAction = new QAction("Save", this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveCurrentFile);
    fileMenu->addAction(saveAction);

    QAction *saveAsAction = new QAction("Save As...", this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveAs);
    fileMenu->addAction(saveAsAction);

    fileMenu->addSeparator();

    QAction *exportAction = new QAction("Export", this);
    connect(exportAction, &QAction::triggered, this, &MainWindow::exportFile);
    fileMenu->addAction(exportAction);

    fileMenu->addSeparator();

    QAction *quitAction = new QAction("Quit", this);
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &MainWindow::close);
    fileMenu->addAction(quitAction);

    // Edit Menu
    QMenu *editMenu = menuBar->addMenu("Edit");
    QAction *undoAction = new QAction("Undo", this);
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, m_editorWidget->editor(), &QTextEdit::undo);
    editMenu->addAction(undoAction);

    QAction *redoAction = new QAction("Redo", this);
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered, m_editorWidget->editor(), &QTextEdit::redo);
    editMenu->addAction(redoAction);

    editMenu->addSeparator();

    QAction *cutAction = new QAction("Cut", this);
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, &QAction::triggered, m_editorWidget->editor(), &QTextEdit::cut);
    editMenu->addAction(cutAction);

    QAction *copyAction = new QAction("Copy", this);
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, m_editorWidget->editor(), &QTextEdit::copy);
    editMenu->addAction(copyAction);

    QAction *pasteAction = new QAction("Paste", this);
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, m_editorWidget->editor(), &QTextEdit::paste);
    editMenu->addAction(pasteAction);

    editMenu->addSeparator();

    QAction *selectAllAction = new QAction("Select All", this);
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    connect(selectAllAction, &QAction::triggered, m_editorWidget->editor(), &QTextEdit::selectAll);
    editMenu->addAction(selectAllAction);

    editMenu->addSeparator();

    // Add Preferences to Edit menu
    QAction *preferencesAction = new QAction("Preferences...", this);
    preferencesAction->setShortcut(QKeySequence::Preferences); // Cmd+, on macOS
    connect(preferencesAction, &QAction::triggered, this, &MainWindow::showPreferences);
    editMenu->addAction(preferencesAction);

#ifdef Q_OS_MAC
    // On macOS, add it to the application menu as well
    menuBar->addAction(preferencesAction);
#endif

    // View Menu
    QMenu *viewMenu = menuBar->addMenu("View");
    QAction *toggleSidebarAction = new QAction("Toggle Sidebar", this);
    toggleSidebarAction->setCheckable(true);
    toggleSidebarAction->setChecked(true);
    toggleSidebarAction->setShortcut(QKeySequence("Ctrl+\\"));
    connect(toggleSidebarAction, &QAction::triggered, this, &MainWindow::toggleSidebar);
    viewMenu->addAction(toggleSidebarAction);

    // Format Menu
    QMenu *formatMenu = menuBar->addMenu("Format");
    QAction *boldAction = new QAction("Bold", this);
    boldAction->setCheckable(true);
    boldAction->setShortcut(QKeySequence::Bold);
    connect(boldAction, &QAction::triggered, this, &MainWindow::setBold);
    formatMenu->addAction(boldAction);

    QAction *italicAction = new QAction("Italic", this);
    italicAction->setCheckable(true);
    italicAction->setShortcut(QKeySequence::Italic);
    connect(italicAction, &QAction::triggered, this, &MainWindow::setItalic);
    formatMenu->addAction(italicAction);

    QAction *underlineAction = new QAction("Underline", this);
    underlineAction->setCheckable(true);
    underlineAction->setShortcut(QKeySequence::Underline);
    connect(underlineAction, &QAction::triggered, this, &MainWindow::setUnderline);
    formatMenu->addAction(underlineAction);
}

void MainWindow::showPreferences()
{
    // Create preferences dialog
    QDialog prefsDialog(this);
    prefsDialog.setWindowTitle("Preferences");
    prefsDialog.setMinimumWidth(400);

    // Create layout
    QVBoxLayout *layout = new QVBoxLayout(&prefsDialog);

    // Theme section
    QGroupBox *themeGroup = new QGroupBox("Theme", &prefsDialog);
    QVBoxLayout *themeLayout = new QVBoxLayout(themeGroup);

    QCheckBox *darkModeCheckbox = new QCheckBox("Dark Mode", themeGroup);
    darkModeCheckbox->setChecked(ThemeManager::instance().isDarkMode());
    connect(darkModeCheckbox, &QCheckBox::toggled, &ThemeManager::instance(), &ThemeManager::setDarkMode);
    themeLayout->addWidget(darkModeCheckbox);

    layout->addWidget(themeGroup);

    // Editor section
    QGroupBox *editorGroup = new QGroupBox("Editor", &prefsDialog);
    QFormLayout *editorLayout = new QFormLayout(editorGroup);

    // Font family
    QFontComboBox *fontCombo = new QFontComboBox(editorGroup);
    fontCombo->setCurrentFont(m_editorWidget->editor()->font());
    connect(fontCombo, &QFontComboBox::currentFontChanged, [this](const QFont &font)
            {
        QFont newFont = m_editorWidget->editor()->font();
        newFont.setFamily(font.family());
        m_editorWidget->editor()->setFont(newFont); });
    editorLayout->addRow("Font:", fontCombo);

    // Font size
    QSpinBox *fontSizeSpinner = new QSpinBox(editorGroup);
    fontSizeSpinner->setRange(8, 72);
    fontSizeSpinner->setValue(m_editorWidget->editor()->font().pointSize());
    connect(fontSizeSpinner, QOverload<int>::of(&QSpinBox::valueChanged), [this](int size)
            {
        QFont newFont = m_editorWidget->editor()->font();
        newFont.setPointSize(size);
        m_editorWidget->editor()->setFont(newFont); });
    editorLayout->addRow("Font Size:", fontSizeSpinner);

    layout->addWidget(editorGroup);

    // Add OK button
    QPushButton *okButton = new QPushButton("OK", &prefsDialog);
    connect(okButton, &QPushButton::clicked, &prefsDialog, &QDialog::accept);
    layout->addWidget(okButton);

    // Show dialog
    prefsDialog.exec();
}

void MainWindow::openFile()
{
    QString appPath = QDir::homePath() + "/Documents/WriteHand";
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Open File"),
                                                    appPath,
                                                    tr("Text Files (*.txt *.md *.rtf *.html *.markdown *.text);;All Files (*)"));

    if (!filePath.isEmpty())
    {
        // Check if the file is outside the app directory
        if (!filePath.startsWith(appPath))
        {
            // Copy the file to the app directory
            QFileInfo fileInfo(filePath);
            QString newPath = appPath + "/" + fileInfo.fileName();

            // Ensure unique filename
            int counter = 1;
            while (QFile::exists(newPath))
            {
                QString baseName = fileInfo.baseName();
                QString suffix = fileInfo.completeSuffix();
                newPath = QString("%1/%2_%3.%4").arg(appPath, baseName).arg(counter).arg(suffix);
                counter++;
            }

            if (QFile::copy(filePath, newPath))
            {
                filePath = newPath;
            }
            else
            {
                QMessageBox::warning(this, tr("Error"), tr("Could not copy file to application directory."));
                return;
            }
        }
        onFileSelected(filePath);
    }
}

void MainWindow::saveAs()
{
    QString defaultPath;
    if (!m_currentFile.isEmpty())
    {
        // Use the directory of the current file
        QFileInfo currentFileInfo(m_currentFile);
        defaultPath = currentFileInfo.absolutePath() + "/" + currentFileInfo.fileName();
    }
    else
    {
        defaultPath = QDir::homePath() + "/Documents/WriteHand";
    }

    QString filePath = QFileDialog::getSaveFileName(this,
                                                    tr("Save As"),
                                                    defaultPath,
                                                    tr("Text Files (*.txt);;Markdown Files (*.md);;Rich Text Files (*.rtf);;HTML Files (*.html);;All Files (*)"));

    if (!filePath.isEmpty())
    {
        // Save the file
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly))
        {
            QTextStream out(&file);
            bool isRichText = filePath.endsWith(".rtf", Qt::CaseInsensitive);
            out << m_editorWidget->content(isRichText);
            file.close();

            // Update current file and window title
            m_currentFile = filePath;
            setWindowTitle("WriteHand - " + QFileInfo(filePath).fileName());

            // Update file tree
            m_fileTreeWidget->selectFile(filePath);
        }
        else
        {
            QMessageBox::warning(this, tr("Error"), tr("Could not save file."));
        }
    }
}

void MainWindow::exportFile()
{
    QString defaultPath;
    if (!m_currentFile.isEmpty())
    {
        // Use the directory of the current file
        QFileInfo currentFileInfo(m_currentFile);
        defaultPath = currentFileInfo.absolutePath() + "/" + currentFileInfo.baseName();
    }
    else
    {
        defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }

    QString filePath = QFileDialog::getSaveFileName(this,
                                                    tr("Export File"),
                                                    defaultPath,
                                                    tr("PDF Files (*.pdf);;Word Documents (*.docx);;Text Files (*.txt);;Markdown Files (*.md);;Rich Text Files (*.rtf);;HTML Files (*.html);;All Files (*)"));

    if (!filePath.isEmpty())
    {
        if (filePath.endsWith(".pdf", Qt::CaseInsensitive))
        {
            // Export as PDF
            QPrinter printer(QPrinter::HighResolution);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(filePath);

            // Set page size and margins
            printer.setPageSize(QPageSize(QPageSize::A4));
            printer.setPageMargins(QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);

            m_editorWidget->editor()->document()->print(&printer);
        }
        else if (filePath.endsWith(".docx", Qt::CaseInsensitive))
        {
            // For DOCX, we'll save as HTML first and let the user know they need to convert it
            QString htmlPath = filePath;
            htmlPath.replace(".docx", ".html");

            QFile file(htmlPath);
            if (file.open(QIODevice::WriteOnly))
            {
                QTextStream out(&file);
                out << m_editorWidget->editor()->toHtml();
                file.close();

                QMessageBox::information(this, tr("Export as Word"),
                                         tr("The document has been exported as HTML. To convert to Word format:\n\n"
                                            "1. Open Microsoft Word or LibreOffice\n"
                                            "2. Open the saved HTML file\n"
                                            "3. Save as DOCX\n\n"
                                            "The HTML file has been saved as: %1")
                                             .arg(htmlPath));
            }
        }
        else
        {
            // Handle other formats as before
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly))
            {
                QTextStream out(&file);
                bool isRichText = filePath.endsWith(".rtf", Qt::CaseInsensitive);
                out << m_editorWidget->content(isRichText);
                file.close();
            }
            else
            {
                QMessageBox::warning(this, tr("Error"), tr("Could not export file."));
            }
        }
    }
}