#include "ColumnView.h"
#include <QtWidgets/QListView>
#include <QtWidgets/QStyle>
#include "ThemeManager.h"

ColumnView::ColumnView(QWidget *parent)
    : QColumnView(parent)
{
  // Only set object name in constructor
  setObjectName("columnView");
}

void ColumnView::setModel(QAbstractItemModel *model)
{
  QColumnView::setModel(model);

  if (model && !m_initialized)
  {
    // Apply theme first
    setStyleSheet(ThemeManager::instance().getStyleSheet("columnView"));

    // Let Qt handle the initialization through the style system
    style()->polish(this);

    m_initialized = true;
  }
}

void ColumnView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
  QColumnView::currentChanged(current, previous);
  if (current.isValid())
  {
    emit itemSelected(current);
  }
}

QAbstractItemView *ColumnView::createColumn(const QModelIndex &rootIndex)
{
  QListView *column = new QListView(this);

  // Apply theme first
  column->setStyleSheet(ThemeManager::instance().getStyleSheet("columnView"));

  // Let Qt handle the initialization through the style system
  style()->polish(column);

  // Set model and root index
  if (model())
  {
    column->setModel(model());
    column->setRootIndex(rootIndex);
  }

  return column;
}