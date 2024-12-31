#include "FontAwesome.h"
#include <QFontDatabase>
#include <QPainter>
#include <QDebug>

// Font Awesome 6 Free Solid icon codes
const QString FontAwesome::EyeSlash = "\uf070";
const QString FontAwesome::Bars = "\uf0c9";
const QString FontAwesome::Bold = "\uf032";
const QString FontAwesome::Italic = "\uf033";
const QString FontAwesome::Underline = "\uf0cd";
const QString FontAwesome::File = "\uf15b";

FontAwesome &FontAwesome::instance()
{
  static FontAwesome instance;
  return instance;
}

FontAwesome::FontAwesome() : m_initialized(false)
{
  initFontAwesome();
}

bool FontAwesome::initFontAwesome()
{
  if (m_initialized)
    return true;

  int fontId = QFontDatabase::addApplicationFont(":/fonts/fa-solid-900.ttf");
  if (fontId == -1)
  {
    qDebug() << "Failed to load Font Awesome font";
    return false;
  }

  QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
  m_font = QFont(family);
  m_initialized = true;
  return true;
}

QIcon FontAwesome::icon(const QString &name, int size)
{
  if (!m_initialized && !initFontAwesome())
    return QIcon();

  QPixmap pixmap(size, size);
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::TextAntialiasing);

  m_font.setPixelSize(size);
  painter.setFont(m_font);

  // Draw normal state
  painter.setPen(QColor("#E0E0E0"));
  painter.drawText(pixmap.rect(), Qt::AlignCenter, name);

  QIcon icon;
  icon.addPixmap(pixmap, QIcon::Normal, QIcon::Off);

  // Draw hover/selected state
  pixmap.fill(Qt::transparent);
  painter.setPen(QColor("#FFFFFF"));
  painter.drawText(pixmap.rect(), Qt::AlignCenter, name);
  icon.addPixmap(pixmap, QIcon::Selected, QIcon::Off);
  icon.addPixmap(pixmap, QIcon::Active, QIcon::Off);

  // Draw disabled state
  pixmap.fill(Qt::transparent);
  painter.setPen(QColor("#808080"));
  painter.drawText(pixmap.rect(), Qt::AlignCenter, name);
  icon.addPixmap(pixmap, QIcon::Disabled, QIcon::Off);

  return icon;
}