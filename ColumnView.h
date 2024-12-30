#pragma once

#include <QtWidgets/QColumnView>

class ColumnView : public QColumnView
{
  Q_OBJECT

public:
  explicit ColumnView(QWidget *parent = nullptr);
  void setModel(QAbstractItemModel *model) override;

signals:
  void itemSelected(const QModelIndex &index);

protected:
  void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
  QAbstractItemView *createColumn(const QModelIndex &rootIndex) override;

private:
  bool m_initialized = false;
  void initializeWidget();
};