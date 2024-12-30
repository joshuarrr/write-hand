#include "FileTreeWidget.h"
#include <QtWidgets/QMenu>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtGui/QDesktopServices>
#include <QtCore/QDateTime>
#include <QtCore/QMimeData>
#include <QtCore/QUrl>
#include <QtWidgets/QListView>
#include <QtWidgets/QSplitter>
#include <QDebug>
#include <QFile>
#include <QTextStream>

FileTreeWidget::FileTreeWidget(QWidget *parent)
    : QWidget(parent), m_model(new QStandardItemModel(this)),
      m_layout(new QHBoxLayout(this)),
      m_locationsView(new QListView(this)),
      m_filesView(new QListView(this)),
      m_archiveSection(nullptr)
{
  // Use direct home path to avoid sandbox
  m_basePath = QDir::homePath() + "/Documents/WriteHand";
  QDir dir(m_basePath);
  if (!dir.exists())
  {
    dir.mkpath(".");
  }

  setupModel();
  setupViews();
  setupConnections();
  createSections();

  // Create a splitter for the views
  QSplitter *viewsSplitter = new QSplitter(Qt::Horizontal, this);
  viewsSplitter->addWidget(m_locationsView);
  viewsSplitter->addWidget(m_filesView);

  // Set initial column widths
  QList<int> sizes;
  sizes << 200 << 200;
  viewsSplitter->setSizes(sizes);

  // Style the splitter
  viewsSplitter->setStyleSheet(
      "QSplitter::handle { "
      "   background-color: #2D2D2D; "
      "   width: 1px; "
      "} "
      "QSplitter::handle:hover { "
      "   background-color: #3D3D3D; "
      "}");

  // Set up main layout
  m_layout->setContentsMargins(0, 0, 0, 0);
  m_layout->setSpacing(0);
  m_layout->addWidget(viewsSplitter);
  setLayout(m_layout);

  // Set size policies
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_locationsView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_filesView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // Style the views
  QString viewStyle =
      "QListView { "
      "    border-right: 1px solid #2D2D2D; "
      "    background-color: #171717; "
      "} "
      "QListView::item { "
      "    padding: 4px 8px; "
      "    border: none; "
      "} "
      "QListView::item:selected { "
      "    background-color: #2D2D2D; "
      "} "
      "QListView::item:hover { "
      "    background-color: #202020; "
      "} "
      "QListView::item:disabled { "
      "    color: #666666; "
      "    padding: 20px 8px; "
      "}";
  m_locationsView->setStyleSheet(viewStyle);
  m_filesView->setStyleSheet(viewStyle);
}

void FileTreeWidget::setupModel()
{
  m_model->setHorizontalHeaderLabels(QStringList() << "Files");
}

void FileTreeWidget::setupViews()
{
  // Set up locations view
  m_locationsView->setModel(m_model);
  m_locationsView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_locationsView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_locationsView->setDragEnabled(false);
  m_locationsView->setAcceptDrops(false);
  m_locationsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_locationsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  // Set up files view
  m_filesView->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
  m_filesView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_filesView->setDragEnabled(true);
  m_filesView->setAcceptDrops(true);
  m_filesView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_filesView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_filesView->setContextMenuPolicy(Qt::CustomContextMenu);
}

void FileTreeWidget::setupConnections()
{
  QFile logFile(m_basePath + "/writehand.log");
  logFile.open(QIODevice::WriteOnly | QIODevice::Append);
  QTextStream log(&logFile);
  log << "\n=== Setting up connections ===\n";
  logFile.close();

  connect(m_locationsView, &QListView::clicked, this, [this](const QModelIndex &index)
          {
    QFile logFile(m_basePath + "/writehand.log");
    logFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream log(&logFile);
    
    log << "\n=== Click event received ===\n";
    
    QStandardItem *item = m_model->itemFromIndex(index);
    if (!item) {
        log << "ERROR: Clicked item is null\n";
        logFile.close();
        return;
    }

    // Debug info
    QString type = item->data(Qt::UserRole + 1).toString();
    QString text = item->text();
    QStandardItem *parentItem = item->parent();
    
    log << "Clicked item details:\n";
    log << "  - Text: " << text << "\n";
    log << "  - Type: " << type << "\n";
    log << "  - Parent: " << (parentItem ? parentItem->text() : "none") << "\n";
    log << "  - Is Archive Section: " << (item == m_archiveSection) << "\n";
    log << "  - Is Favorites Section: " << (item == m_favoritesSection) << "\n";
    log << "  - Is Smart Folders Section: " << (item == m_smartFoldersSection) << "\n";
    log << "  - Is Tags Section: " << (item == m_tagsSection) << "\n";
    log << "  - Is Locations Section: " << (item == m_locationsSection) << "\n";

    // Select the item
    m_locationsView->setCurrentIndex(index);

    // Handle sections by type
    QString sectionType = item->data(Qt::UserRole + 1).toString();
    log << "Handling section type: " << sectionType << "\n";

    if (sectionType == "archive") {
        log << "Updating archive view...\n";
        logFile.close();
        updateArchiveView();
        return;
    } else if (sectionType == "favorites") {
        log << "Updating favorites view...\n";
        logFile.close();
        updateFavoritesView();
        return;
    } else if (sectionType == "smartfolders") {
        if (item == m_smartFoldersSection) {
            log << "Showing smart folders empty state...\n";
            logFile.close();
            showEmptyState("Create a smart folder to filter files by type");
        } else {
            log << "Updating smart folder view...\n";
            logFile.close();
            updateSmartFolderView(item);
        }
        return;
    } else if (sectionType == "tags") {
        log << "Showing tags empty state...\n";
        logFile.close();
        showEmptyState("Add #tags to your files to group them");
        return;
    } else if (sectionType == "locations") {
        if (item == m_locationsSection) {
            log << "Showing locations empty state...\n";
            logFile.close();
            showEmptyState("Select a location to view files");
        } else {
            log << "Updating files view for location: " << item->text() << "\n";
            logFile.close();
            updateFilesView(item);
        }
        return;
    }

    // Handle items under sections
    if (parentItem) {
        if (type == "location") {
            log << "Updating files view for location: " << item->text() << "\n";
            logFile.close();
            updateFilesView(item);
        } else if (type == "favorite") {
            QString path = item->data(Qt::UserRole).toString();
            QFileInfo fileInfo(path);
            if (fileInfo.isDir()) {
                log << "Updating files view for favorite directory: " << item->text() << "\n";
                logFile.close();
                updateFilesView(item);
            } else {
                log << "Selected favorite file: " << item->text() << "\n";
                logFile.close();
                emit fileSelected(path);
            }
        } else if (type == "smartfolder") {
            log << "Updating smart folder view: " << item->text() << "\n";
            logFile.close();
            updateSmartFolderView(item);
        }
    }

    logFile.close(); });

  connect(m_filesView, &QListView::clicked, this, [this](const QModelIndex &index)
          {
    QFile logFile(m_basePath + "/writehand.log");
    logFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream log(&logFile);
    
    log << "\n=== Files view click event received ===\n";
    
    QStandardItem *item = qobject_cast<QStandardItemModel*>(m_filesView->model())->itemFromIndex(index);
    if (!item) {
        log << "ERROR: Clicked item is null\n";
        logFile.close();
        return;
    }

    QString type = item->data(Qt::UserRole + 1).toString();
    QString path = item->data(Qt::UserRole).toString();
    
    log << "Clicked file details:\n";
    log << "  - Text: " << item->text() << "\n";
    log << "  - Type: " << type << "\n";
    log << "  - Path: " << path << "\n";
    
    if (type == "file") {
        log << "Emitting fileSelected signal\n";
        emit fileSelected(path);
        m_filesView->setCurrentIndex(index);
    }
    
    logFile.close(); });

  connect(m_filesView, &QWidget::customContextMenuRequested, this, &FileTreeWidget::handleContextMenu);
}

void FileTreeWidget::updateFilesView(QStandardItem *locationItem)
{
  QFile logFile(m_basePath + "/writehand.log");
  logFile.open(QIODevice::WriteOnly | QIODevice::Append);
  QTextStream log(&logFile);

  log << "\n=== Updating Files View ===\n";
  QString path = locationItem->data(Qt::UserRole).toString();
  log << "Location path: " << path << "\n";

  QStandardItemModel *filesModel = new QStandardItemModel(m_filesView);

  QDir dir(path);
  QStringList filters;
  filters << "*.txt" << "*.md" << "*.rtf" << "*.html" << "*.markdown" << "*.text";
  QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time);

  log << "Found " << files.count() << " files in location:\n";
  for (const QFileInfo &fileInfo : files)
  {
    QStandardItem *fileItem = new QStandardItem(fileInfo.fileName());
    fileItem->setData(fileInfo.filePath(), Qt::UserRole);
    fileItem->setData("file", Qt::UserRole + 1);
    fileItem->setFlags(fileItem->flags() | Qt::ItemIsEditable);
    filesModel->appendRow(fileItem);
    log << "  - Added file: " << fileInfo.fileName() << " (path: " << fileInfo.filePath() << ")\n";
  }

  log << "Setting new model for files view\n";
  m_filesView->setModel(filesModel);
  log << "Files view update complete\n";
  logFile.close();
}

void FileTreeWidget::updateSmartFolderView(QStandardItem *smartFolderItem)
{
  QFile logFile(m_basePath + "/writehand.log");
  logFile.open(QIODevice::WriteOnly | QIODevice::Append);
  QTextStream log(&logFile);

  log << "\n=== Updating Smart Folder View ===\n";
  QString pattern = smartFolderItem->data(Qt::UserRole).toString();
  log << "Smart folder pattern: " << pattern << "\n";

  QStandardItemModel *filesModel = new QStandardItemModel(m_filesView);

  // Add files from all locations that match the pattern
  int totalFiles = 0;
  for (auto it = m_locationItems.begin(); it != m_locationItems.end(); ++it)
  {
    QString path = it.value()->data(Qt::UserRole).toString();
    log << "Searching in location: " << path << "\n";

    QDir dir(path);
    QStringList filters = pattern.split(",");
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time);

    log << "Found " << files.count() << " matching files\n";
    totalFiles += files.count();

    for (const QFileInfo &fileInfo : files)
    {
      QStandardItem *fileItem = new QStandardItem(fileInfo.fileName());
      fileItem->setData(fileInfo.filePath(), Qt::UserRole);
      fileItem->setData("file", Qt::UserRole + 1);
      fileItem->setFlags(fileItem->flags() | Qt::ItemIsEditable);
      filesModel->appendRow(fileItem);
      log << "  - Added file: " << fileInfo.fileName() << " (path: " << fileInfo.filePath() << ")\n";
    }
  }

  if (totalFiles == 0)
  {
    log << "No matching files found, showing empty state\n";
    logFile.close();
    showEmptyState("No files match this smart folder");
    return;
  }

  log << "Setting new model for files view\n";
  m_filesView->setModel(filesModel);
  log << "Smart folder view update complete\n";
  logFile.close();
}

void FileTreeWidget::updateArchiveView()
{
  QFile logFile(m_basePath + "/writehand.log");
  logFile.open(QIODevice::WriteOnly | QIODevice::Append);
  QTextStream log(&logFile);

  log << "\n=== Updating Archive View ===\n";
  QString archivePath = m_basePath + "/Archive";
  log << "Archive path: " << archivePath << "\n";

  QStandardItemModel *filesModel = new QStandardItemModel(m_filesView);

  QDir dir(archivePath);
  log << "Archive directory exists: " << dir.exists() << "\n";

  if (dir.exists())
  {
    QStringList filters;
    filters << "*.txt" << "*.md" << "*.rtf" << "*.html" << "*.markdown" << "*.text";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time);

    log << "Found " << files.count() << " files in archive:\n";
    for (const QFileInfo &file : files)
    {
      log << "  - " << file.fileName() << "\n";
    }

    if (files.isEmpty())
    {
      log << "No archived files found, showing empty state\n";
      logFile.close();
      showEmptyState("No archived files");
      return;
    }

    for (const QFileInfo &fileInfo : files)
    {
      QStandardItem *fileItem = new QStandardItem(fileInfo.fileName());
      fileItem->setData(fileInfo.filePath(), Qt::UserRole);
      fileItem->setData("file", Qt::UserRole + 1);
      fileItem->setFlags(fileItem->flags() | Qt::ItemIsEditable);
      filesModel->appendRow(fileItem);
      log << "Added archived file to model: " << fileInfo.fileName() << "\n";
    }
  }
  else
  {
    log << "Archive directory does not exist, showing empty state\n";
    logFile.close();
    showEmptyState("No archived files");
    return;
  }

  log << "Setting new model for files view\n";
  m_filesView->setModel(filesModel);
  log << "Archive view update complete\n";
  logFile.close();
}

void FileTreeWidget::createSections()
{
  QFile logFile(m_basePath + "/writehand.log");
  logFile.open(QIODevice::WriteOnly | QIODevice::Append);
  QTextStream log(&logFile);

  log << "\n=== Creating sections ===\n";

  // Clear any existing items
  m_model->clear();
  m_model->setHorizontalHeaderLabels(QStringList() << "Files");

  // Create main sections
  m_locationsSection = new QStandardItem("Locations");
  m_favoritesSection = new QStandardItem("Favorites");
  m_smartFoldersSection = new QStandardItem("Smart Folders");
  m_tagsSection = new QStandardItem("Tags");
  m_archiveSection = nullptr; // Explicitly set to nullptr

  log << "Created base sections\n";

  // Set sections non-editable
  QList<QStandardItem *> sections = {m_locationsSection, m_favoritesSection, m_smartFoldersSection, m_tagsSection};
  for (auto section : sections)
  {
    section->setFlags(section->flags() & ~Qt::ItemIsEditable);
    section->setData("section", Qt::UserRole);
    section->setData(section->text().toLower().replace(" ", ""), Qt::UserRole + 1); // e.g. "locations", "favorites", etc.
    m_model->appendRow(section);
    log << "Added section: " << section->text() << " with type: " << section->data(Qt::UserRole + 1).toString() << "\n";
  }

  // Add default locations with the correct path
  addLocation("Documents", m_basePath);

  // Add default smart folders
  addSmartFolder("Recents", "*.{txt,md,rtf,html,markdown,text}");

  // Check for archived files and create archive section if needed
  QString archivePath = m_basePath + "/Archive";
  QDir archiveDir(archivePath);
  log << "\n=== Archive Setup ===\n";
  log << "Archive path: " << archivePath << "\n";
  log << "Archive dir exists: " << archiveDir.exists() << "\n";

  if (archiveDir.exists())
  {
    QStringList filters;
    filters << "*.txt" << "*.md" << "*.rtf" << "*.html" << "*.markdown" << "*.text";
    QFileInfoList files = archiveDir.entryInfoList(filters, QDir::Files, QDir::Time);
    log << "Found " << files.count() << " archived files:\n";
    for (const QFileInfo &file : files)
    {
      log << "  - " << file.fileName() << "\n";
    }

    if (!files.isEmpty())
    {
      m_archiveSection = new QStandardItem("Archive");
      m_archiveSection->setFlags(m_archiveSection->flags() & ~Qt::ItemIsEditable);
      m_archiveSection->setData("section", Qt::UserRole);
      m_archiveSection->setData("archive", Qt::UserRole + 1);
      m_model->appendRow(m_archiveSection);
      log << "Created and added Archive section to model\n";
      log << "Archive section types - UserRole: " << m_archiveSection->data(Qt::UserRole).toString()
          << ", UserRole+1: " << m_archiveSection->data(Qt::UserRole + 1).toString() << "\n";
    }
    else
    {
      log << "No archived files found, skipping Archive section\n";
    }
  }
  else
  {
    log << "Archive directory does not exist\n";
  }

  log << "Archive section pointer: " << m_archiveSection << "\n";
  log << "=== Section creation complete ===\n";
  logFile.close();

  // Select Documents by default and show its files
  if (m_locationItems.contains("Documents"))
  {
    QModelIndex documentsIndex = m_locationItems["Documents"]->index();
    m_locationsView->setCurrentIndex(documentsIndex);
    updateFilesView(m_locationItems["Documents"]);
  }
}

void FileTreeWidget::selectFile(const QString &filePath)
{
  // Find the file in the Documents section
  QStandardItem *documentsItem = m_locationItems["Documents"];
  if (!documentsItem)
    return;

  QString fileName = QFileInfo(filePath).fileName();
  QStandardItemModel *filesModel = qobject_cast<QStandardItemModel *>(m_filesView->model());
  if (!filesModel)
    return;

  // Find and select the file in the files view
  for (int i = 0; i < filesModel->rowCount(); ++i)
  {
    QStandardItem *item = filesModel->item(i);
    if (item && item->data(Qt::UserRole).toString() == filePath)
    {
      m_filesView->setCurrentIndex(filesModel->indexFromItem(item));
      break;
    }
  }
}

void FileTreeWidget::createNewFile()
{
  QString filePath = getNextFileName();
  QFile file(filePath);
  if (file.open(QIODevice::WriteOnly))
  {
    file.close();
    emit fileCreated(filePath);

    // Update the files view if Documents is selected
    QModelIndex currentIndex = m_locationsView->currentIndex();
    if (currentIndex.isValid())
    {
      QStandardItem *item = m_model->itemFromIndex(currentIndex);
      if (item && item->data(Qt::UserRole + 1).toString() == "location")
      {
        updateFilesView(item);
      }
    }
  }
}

QString FileTreeWidget::getNextFileName()
{
  int counter = 1;
  QString baseName = m_basePath + "/untitled";
  QString extension = ".txt";
  QString filePath = baseName + extension;

  while (QFile::exists(filePath))
  {
    filePath = QString("%1 %2%3").arg(baseName).arg(counter).arg(extension);
    counter++;
  }

  return filePath;
}

void FileTreeWidget::handleContextMenu(const QPoint &pos)
{
  QModelIndex index = m_filesView->indexAt(pos);
  QStandardItemModel *filesModel = qobject_cast<QStandardItemModel *>(m_filesView->model());
  QStandardItem *item = filesModel ? filesModel->itemFromIndex(index) : nullptr;

  QMenu menu(this);

  if (!item)
  {
    QAction *newFileAction = menu.addAction("New File");
    connect(newFileAction, &QAction::triggered, this, &FileTreeWidget::createNewFile);
  }
  else
  {
    QString type = item->data(Qt::UserRole + 1).toString();
    if (type == "file")
    {
      QAction *renameAction = menu.addAction("Rename");
      QAction *deleteAction = menu.addAction("Delete");
      QAction *archiveAction = menu.addAction("Move to Archive");

      connect(renameAction, &QAction::triggered, [this, index]()
              { m_filesView->edit(index); });

      connect(deleteAction, &QAction::triggered, [this, item]()
              {
                       QString filePath = item->data(Qt::UserRole).toString();
                       QFile file(filePath);
                       if (file.remove())
                       {
                           emit fileDeleted(filePath);
                           updateFilesView(m_locationItems["Documents"]);
                       } });

      connect(archiveAction, &QAction::triggered, [this, item]()
              {
                       QString oldPath = item->data(Qt::UserRole).toString();
                       QString fileName = QFileInfo(oldPath).fileName();
                       QString archivePath = m_basePath + "/Archive";
                       
                       // Create Archive directory if it doesn't exist
                       QDir().mkpath(archivePath);
                       
                       QString newPath = archivePath + "/" + fileName;
                       QFile file(oldPath);
                       
                       if (file.rename(newPath))
                       {
                           updateFilesView(m_locationItems["Documents"]);
                       } });
    }
  }

  menu.exec(m_filesView->viewport()->mapToGlobal(pos));
}

void FileTreeWidget::addLocation(const QString &name, const QString &path)
{
  QStandardItem *item = new QStandardItem(name);
  item->setData(path, Qt::UserRole);
  item->setData("location", Qt::UserRole + 1);
  item->setFlags(item->flags() & ~Qt::ItemIsEditable);
  m_locationItems[name] = item;
  m_locationsSection->appendRow(item);
}

void FileTreeWidget::addSmartFolder(const QString &name, const QString &filterPattern)
{
  QStandardItem *item = new QStandardItem(name);
  item->setData(filterPattern, Qt::UserRole);
  item->setData("smartfolder", Qt::UserRole + 1);
  item->setFlags(item->flags() & ~Qt::ItemIsEditable);
  m_smartFolderItems[name] = item;
  m_smartFoldersSection->appendRow(item);
}

void FileTreeWidget::addFavorite(const QString &name, const QString &path)
{
  if (m_favoriteItems.contains(path))
    return;

  QStandardItem *item = new QStandardItem(name);
  item->setData(path, Qt::UserRole);
  item->setData("favorite", Qt::UserRole + 1);
  m_favoriteItems[path] = item;
  m_favoritesSection->appendRow(item);
}

void FileTreeWidget::showEmptyState(const QString &message)
{
  QStandardItemModel *model = new QStandardItemModel(m_filesView);
  QStandardItem *emptyItem = new QStandardItem(message);
  emptyItem->setFlags(emptyItem->flags() & ~Qt::ItemIsEnabled);
  emptyItem->setData("empty", Qt::UserRole + 1);
  model->appendRow(emptyItem);
  m_filesView->setModel(model);
}

void FileTreeWidget::updateFavoritesView()
{
  QStandardItemModel *filesModel = new QStandardItemModel(m_filesView);

  if (m_favoriteItems.isEmpty())
  {
    showEmptyState("Drag folders and files here for quick access");
    return;
  }

  for (auto it = m_favoriteItems.begin(); it != m_favoriteItems.end(); ++it)
  {
    QStandardItem *item = it.value();
    if (item->data(Qt::UserRole + 1).toString() != "placeholder")
    {
      filesModel->appendRow(item->clone());
    }
  }

  m_filesView->setModel(filesModel);
}