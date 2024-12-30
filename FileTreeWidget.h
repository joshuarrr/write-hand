#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QStandardItemModel>
#include <QtCore/QMap>

class FileTreeWidget : public QWidget
{
  Q_OBJECT

public:
  explicit FileTreeWidget(QWidget *parent = nullptr);
  void selectFile(const QString &filePath);

signals:
  void fileSelected(const QString &filePath);
  void fileCreated(const QString &filePath);
  void fileRenamed(const QString &oldPath, const QString &newPath);
  void fileDeleted(const QString &filePath);

public slots:
  void createNewFile();
  void refreshModel();

private:
  void setupModel();
  void setupView();
  void setupConnections();
  void createSections();
  void addLocation(const QString &name, const QString &path);
  void addFavorite(const QString &name, const QString &path);
  void addSmartFolder(const QString &name, const QString &filterPattern);
  void addTagSection();
  QString getNextFileName();
  void handleItemClicked(const QModelIndex &index);
  void handleItemDoubleClicked(const QModelIndex &index);
  void handleContextMenu(const QPoint &pos);

  QTreeView *m_treeView;
  QStandardItemModel *m_model;
  QVBoxLayout *m_layout;
  QString m_basePath;

  // Section items for easy access
  QStandardItem *m_locationsSection;
  QStandardItem *m_favoritesSection;
  QStandardItem *m_smartFoldersSection;
  QStandardItem *m_tagsSection;
  QStandardItem *m_archiveSection;

  // Keep track of special items
  QMap<QString, QStandardItem *> m_locationItems;
  QMap<QString, QStandardItem *> m_favoriteItems;
  QMap<QString, QStandardItem *> m_smartFolderItems;
};