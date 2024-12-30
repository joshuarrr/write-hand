#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListView>
#include <QtGui/QStandardItemModel>
#include <QtCore/QMap>

class FileTreeWidget : public QWidget
{
  Q_OBJECT

public:
  explicit FileTreeWidget(QWidget *parent = nullptr);
  void selectFile(const QString &filePath);
  void refreshModel();

signals:
  void fileSelected(const QString &filePath);
  void fileCreated(const QString &filePath);
  void fileRenamed(const QString &oldPath, const QString &newPath);
  void fileDeleted(const QString &filePath);

private slots:
  void handleContextMenu(const QPoint &pos);

public slots:
  void createNewFile();

private:
  void setupModel();
  void setupViews();
  void setupConnections();
  void createSections();
  void addLocation(const QString &name, const QString &path);
  void addFavorite(const QString &name, const QString &path);
  void addSmartFolder(const QString &name, const QString &filterPattern);
  QString getNextFileName();
  void updateFilesView(QStandardItem *locationItem);
  void updateSmartFolderView(QStandardItem *smartFolderItem);
  void updateArchiveView();
  void updateFavoritesView();
  void showEmptyState(const QString &message);

  QListView *m_locationsView;
  QListView *m_filesView;
  QStandardItemModel *m_model;
  QHBoxLayout *m_layout;
  QString m_basePath;

  QStandardItem *m_locationsSection;
  QStandardItem *m_favoritesSection;
  QStandardItem *m_smartFoldersSection;
  QStandardItem *m_tagsSection;
  QStandardItem *m_archiveSection;

  QMap<QString, QStandardItem *> m_locationItems;
  QMap<QString, QStandardItem *> m_favoriteItems;
  QMap<QString, QStandardItem *> m_smartFolderItems;
};