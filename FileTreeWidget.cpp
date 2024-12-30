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

FileTreeWidget::FileTreeWidget(QWidget *parent)
    : QWidget(parent), m_treeView(new QTreeView(this)), m_model(new QStandardItemModel(this)),
      m_layout(new QVBoxLayout(this)), m_archiveSection(nullptr)
{
  // Use direct home path to avoid sandbox
  m_basePath = QDir::homePath() + "/Documents/WriteHand";
  QDir dir(m_basePath);
  if (!dir.exists())
  {
    dir.mkpath(".");
  }

  setupModel();
  setupView();
  setupConnections();
  createSections();

  m_layout->setContentsMargins(0, 0, 0, 0);
  m_layout->addWidget(m_treeView);
  setLayout(m_layout);
}

void FileTreeWidget::setupModel()
{
  m_model->setHorizontalHeaderLabels(QStringList() << "Files");
}

void FileTreeWidget::setupView()
{
  m_treeView->setModel(m_model);
  m_treeView->setHeaderHidden(true);
  m_treeView->setDragEnabled(true);
  m_treeView->setDropIndicatorShown(true);
  m_treeView->setAcceptDrops(true);
  m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
  m_treeView->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);
  m_treeView->setAnimated(true);
  m_treeView->setIndentation(15);

  // Set selection style
  m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void FileTreeWidget::setupConnections()
{
  connect(m_treeView, &QTreeView::clicked, this, &FileTreeWidget::handleItemClicked);
  connect(m_treeView, &QTreeView::doubleClicked, this, &FileTreeWidget::handleItemDoubleClicked);
  connect(m_treeView, &QTreeView::customContextMenuRequested, this, &FileTreeWidget::handleContextMenu);
  connect(m_model, &QStandardItemModel::itemChanged, this, [this](QStandardItem *item)
          {
    if (item->data(Qt::UserRole + 1).toString() == "file")
    {
      QString oldPath = item->data(Qt::UserRole).toString();
      QString newName = item->text();
      QFileInfo fileInfo(oldPath);
      QString newPath = fileInfo.dir().filePath(newName);
      
      if (oldPath != newPath)
      {
        QFile file(oldPath);
        if (file.rename(newPath))
        {
          item->setData(newPath, Qt::UserRole);
          emit fileRenamed(oldPath, newPath);
        }
        else
        {
          // If rename fails, revert the item text
          item->setText(fileInfo.fileName());
        }
      }
    } });
}

void FileTreeWidget::createSections()
{
  // Create main sections
  m_locationsSection = new QStandardItem("Locations");
  m_favoritesSection = new QStandardItem("Favorites");
  m_smartFoldersSection = new QStandardItem("Smart Folders");
  m_tagsSection = new QStandardItem("Tags");

  // Set sections non-editable and with custom styling
  QList<QStandardItem *> sections = {m_locationsSection, m_favoritesSection, m_smartFoldersSection, m_tagsSection};
  for (auto section : sections)
  {
    section->setFlags(section->flags() & ~Qt::ItemIsEditable);
    section->setData("section", Qt::UserRole);
    m_model->appendRow(section);
  }

  // Add default locations with the correct path
  addLocation("Documents", m_basePath);

  // Add "Drag folders and files here for quick access" placeholder
  QStandardItem *placeholder = new QStandardItem("Drag folders and files here for quick access");
  placeholder->setFlags(placeholder->flags() & ~Qt::ItemIsEditable);
  placeholder->setData("placeholder", Qt::UserRole);
  m_favoritesSection->appendRow(placeholder);

  // Add default smart folders
  addSmartFolder("Recents", "*.{txt,md,rtf,html,markdown,text}");

  // Add "Write #tags to group files" placeholder
  QStandardItem *tagsPlaceholder = new QStandardItem("Write #tags to group files");
  tagsPlaceholder->setFlags(tagsPlaceholder->flags() & ~Qt::ItemIsEditable);
  tagsPlaceholder->setData("placeholder", Qt::UserRole);
  m_tagsSection->appendRow(tagsPlaceholder);

  // Check for archived files and create archive section if needed
  QString archivePath = m_basePath + "/Archive";
  QDir archiveDir(archivePath);
  if (archiveDir.exists())
  {
    QStringList filters;
    filters << "*.txt" << "*.md" << "*.rtf" << "*.html" << "*.markdown" << "*.text";
    QFileInfoList files = archiveDir.entryInfoList(filters, QDir::Files, QDir::Time);

    if (!files.isEmpty())
    {
      m_archiveSection = new QStandardItem("Archive");
      m_archiveSection->setFlags(m_archiveSection->flags() & ~Qt::ItemIsEditable);
      m_archiveSection->setData("section", Qt::UserRole);
      m_model->appendRow(m_archiveSection);

      // Add archived files
      for (const QFileInfo &fileInfo : files)
      {
        QStandardItem *fileItem = new QStandardItem(fileInfo.fileName());
        fileItem->setData(fileInfo.filePath(), Qt::UserRole);
        fileItem->setData("file", Qt::UserRole + 1);
        fileItem->setFlags(fileItem->flags() | Qt::ItemIsEditable);
        m_archiveSection->appendRow(fileItem);
      }
    }
  }

  // Expand all sections and the Documents location
  m_treeView->expandAll();
  if (m_locationItems.contains("Documents"))
  {
    m_treeView->expand(m_locationItems["Documents"]->index());
  }
}

void FileTreeWidget::addLocation(const QString &name, const QString &path)
{
  QStandardItem *item = new QStandardItem(name);
  item->setData(path, Qt::UserRole);
  item->setData("location", Qt::UserRole + 1);
  item->setFlags(item->flags() & ~Qt::ItemIsEditable); // Make location non-editable
  m_locationItems[name] = item;
  m_locationsSection->appendRow(item);

  // Add files under this location
  QDir dir(path);
  QStringList filters;
  filters << "*.txt" << "*.md" << "*.rtf" << "*.html" << "*.markdown" << "*.text";
  QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time);

  for (const QFileInfo &fileInfo : files)
  {
    QStandardItem *fileItem = new QStandardItem(fileInfo.fileName());
    fileItem->setData(fileInfo.filePath(), Qt::UserRole);
    fileItem->setData("file", Qt::UserRole + 1);
    fileItem->setFlags(fileItem->flags() | Qt::ItemIsEditable); // Make file items editable
    item->appendRow(fileItem);
  }
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

void FileTreeWidget::addSmartFolder(const QString &name, const QString &filterPattern)
{
  QStandardItem *item = new QStandardItem(name);
  item->setData(filterPattern, Qt::UserRole);
  item->setData("smartfolder", Qt::UserRole + 1);
  m_smartFolderItems[name] = item;
  m_smartFoldersSection->appendRow(item);
}

void FileTreeWidget::handleItemClicked(const QModelIndex &index)
{
  QStandardItem *item = m_model->itemFromIndex(index);
  if (!item)
    return;

  QString type = item->data(Qt::UserRole + 1).toString();
  QString path = item->data(Qt::UserRole).toString();

  if (type == "file")
  {
    emit fileSelected(path);
  }
}

void FileTreeWidget::handleItemDoubleClicked(const QModelIndex &index)
{
  QStandardItem *item = m_model->itemFromIndex(index);
  if (!item)
    return;

  QString type = item->data(Qt::UserRole + 1).toString();

  if (type == "location" || type == "favorite")
  {
    // Always use m_basePath which is set to the non-sandboxed path
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_basePath));
  }
}

void FileTreeWidget::handleContextMenu(const QPoint &pos)
{
  QModelIndex index = m_treeView->indexAt(pos);
  QStandardItem *item = m_model->itemFromIndex(index);

  QMenu menu(this);

  if (!item || item->data(Qt::UserRole + 1).toString() == "section")
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
              { m_treeView->edit(index); });
      connect(deleteAction, &QAction::triggered, [this, item]()
              {
                // Implement delete functionality
              });
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
                  // Create archive section if it doesn't exist
                  if (!m_archiveSection)
                  {
                    m_archiveSection = new QStandardItem("Archive");
                    m_archiveSection->setFlags(m_archiveSection->flags() & ~Qt::ItemIsEditable);
                    m_archiveSection->setData("section", Qt::UserRole);
                    // Add archive section at the bottom
                    m_model->appendRow(m_archiveSection);
                  }
                  
                  // Add file to archive section
                  QStandardItem *archivedItem = new QStandardItem(fileName);
                  archivedItem->setData(newPath, Qt::UserRole);
                  archivedItem->setData("file", Qt::UserRole + 1);
                  archivedItem->setFlags(archivedItem->flags() | Qt::ItemIsEditable);
                  m_archiveSection->appendRow(archivedItem);
                  
                  // Remove file from its original location
                  item->parent()->removeRow(item->row());
                  
                  // Expand archive section
                  m_treeView->expand(m_model->indexFromItem(m_archiveSection));
                } });
    }
  }

  menu.exec(m_treeView->viewport()->mapToGlobal(pos));
}

void FileTreeWidget::createNewFile()
{
  QString filePath = getNextFileName();
  QFile file(filePath);
  if (file.open(QIODevice::WriteOnly))
  {
    file.close();
    emit fileCreated(filePath);
    refreshModel();
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

void FileTreeWidget::selectFile(const QString &filePath)
{
  // Find and select the file in the tree
  QList<QStandardItem *> items = m_model->findItems(QFileInfo(filePath).fileName(), Qt::MatchRecursive | Qt::MatchExactly);
  for (QStandardItem *item : items)
  {
    if (item->data(Qt::UserRole).toString() == filePath)
    {
      m_treeView->setCurrentIndex(item->index());
      m_treeView->scrollTo(item->index()); // Ensure the item is visible
      break;
    }
  }
}

void FileTreeWidget::refreshModel()
{
  // Refresh the file list in each location
  for (auto it = m_locationItems.begin(); it != m_locationItems.end(); ++it)
  {
    QStandardItem *locationItem = it.value();
    QString path = locationItem->data(Qt::UserRole).toString();

    // Clear existing items
    locationItem->removeRows(0, locationItem->rowCount());

    // Add files from the location
    QDir dir(path);
    QStringList filters;
    filters << "*.txt" << "*.md" << "*.rtf" << "*.html" << "*.markdown" << "*.text";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time);

    for (const QFileInfo &fileInfo : files)
    {
      QStandardItem *fileItem = new QStandardItem(fileInfo.fileName());
      fileItem->setData(fileInfo.filePath(), Qt::UserRole);
      fileItem->setData("file", Qt::UserRole + 1);
      fileItem->setFlags(fileItem->flags() | Qt::ItemIsEditable);
      locationItem->appendRow(fileItem);
    }
  }

  // Check for archived files and create archive section if needed
  QString archivePath = m_basePath + "/Archive";
  QDir archiveDir(archivePath);
  if (archiveDir.exists())
  {
    QStringList filters;
    filters << "*.txt" << "*.md" << "*.rtf" << "*.html" << "*.markdown" << "*.text";
    QFileInfoList files = archiveDir.entryInfoList(filters, QDir::Files, QDir::Time);

    if (!files.isEmpty())
    {
      // Create archive section if it doesn't exist
      if (!m_archiveSection)
      {
        m_archiveSection = new QStandardItem("Archive");
        m_archiveSection->setFlags(m_archiveSection->flags() & ~Qt::ItemIsEditable);
        m_archiveSection->setData("section", Qt::UserRole);
        // Add archive section at the bottom
        m_model->appendRow(m_archiveSection);
      }
      else
      {
        m_archiveSection->removeRows(0, m_archiveSection->rowCount());
      }

      // Add archived files
      for (const QFileInfo &fileInfo : files)
      {
        QStandardItem *fileItem = new QStandardItem(fileInfo.fileName());
        fileItem->setData(fileInfo.filePath(), Qt::UserRole);
        fileItem->setData("file", Qt::UserRole + 1);
        fileItem->setFlags(fileItem->flags() | Qt::ItemIsEditable);
        m_archiveSection->appendRow(fileItem);
      }
    }
    else if (m_archiveSection)
    {
      // Remove archive section if it exists but there are no archived files
      m_model->removeRow(m_model->indexFromItem(m_archiveSection).row());
      m_archiveSection = nullptr;
    }
  }

  // Refresh smart folders
  for (auto it = m_smartFolderItems.begin(); it != m_smartFolderItems.end(); ++it)
  {
    QStandardItem *smartFolderItem = it.value();
    QString pattern = smartFolderItem->data(Qt::UserRole).toString();

    // Clear existing items
    smartFolderItem->removeRows(0, smartFolderItem->rowCount());

    // Add files from all locations that match the pattern
    for (auto locIt = m_locationItems.begin(); locIt != m_locationItems.end(); ++locIt)
    {
      QString path = locIt.value()->data(Qt::UserRole).toString();
      QDir dir(path);
      QStringList filters = pattern.split(",");
      QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time);

      for (const QFileInfo &fileInfo : files)
      {
        QStandardItem *fileItem = new QStandardItem(fileInfo.fileName());
        fileItem->setData(fileInfo.filePath(), Qt::UserRole);
        fileItem->setData("file", Qt::UserRole + 1);
        smartFolderItem->appendRow(fileItem);
      }
    }
  }

  // Expand all sections
  m_treeView->expandAll();
}