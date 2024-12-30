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

  // Set up layout
  m_layout->setContentsMargins(0, 0, 0, 0);
  m_layout->setSpacing(0);
  m_layout->addWidget(m_locationsView);
  m_layout->addWidget(m_filesView);
  setLayout(m_layout);

  // Set size policies
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_locationsView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  m_filesView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  // Set fixed widths
  m_locationsView->setFixedWidth(200);
  m_filesView->setFixedWidth(200);

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
  connect(m_locationsView, &QListView::clicked, this, [this](const QModelIndex &index)
          {
        QStandardItem *item = m_model->itemFromIndex(index);
        if (!item)
            return;

        // If clicking on Documents, show its files
        if (item->data(Qt::UserRole + 1).toString() == "location") {
            updateFilesView(item);
            
            // Select the item
            m_locationsView->setCurrentIndex(index);
        } });

  connect(m_filesView, &QListView::clicked, this, [this](const QModelIndex &index)
          {
        QStandardItem *item = m_model->itemFromIndex(index);
        if (item && item->data(Qt::UserRole + 1).toString() == "file") {
            emit fileSelected(item->data(Qt::UserRole).toString());
            
            // Select the item
            m_filesView->setCurrentIndex(index);
        } });

  connect(m_filesView, &QWidget::customContextMenuRequested, this, &FileTreeWidget::handleContextMenu);
}

void FileTreeWidget::updateFilesView(QStandardItem *locationItem)
{
  QString path = locationItem->data(Qt::UserRole).toString();
  QStandardItemModel *filesModel = new QStandardItemModel(m_filesView);

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
    filesModel->appendRow(fileItem);
  }

  m_filesView->setModel(filesModel);
}

void FileTreeWidget::createSections()
{
  // Clear any existing items
  m_model->clear();
  m_model->setHorizontalHeaderLabels(QStringList() << "Files");

  // Create main sections
  m_locationsSection = new QStandardItem("Locations");
  m_favoritesSection = new QStandardItem("Favorites");
  m_smartFoldersSection = new QStandardItem("Smart Folders");
  m_tagsSection = new QStandardItem("Tags");

  // Set sections non-editable
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
  QStandardItem *placeholder = new QStandardItem("Drag folders and files here...");
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
    }
  }

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