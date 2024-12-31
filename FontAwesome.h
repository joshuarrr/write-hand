#pragma once

#include <QObject>
#include <QIcon>
#include <QFont>
#include <QString>

class FontAwesome : public QObject
{
  Q_OBJECT

public:
  static FontAwesome &instance();

  // Icon codes from Font Awesome
  static const QString EyeSlash;
  static const QString Bars; // hamburger menu
  static const QString Bold;
  static const QString Italic;
  static const QString Underline;
  static const QString File; // for new file

  QIcon icon(const QString &name, int size = 24);
  bool initFontAwesome();

private:
  FontAwesome();
  ~FontAwesome() = default;
  FontAwesome(const FontAwesome &) = delete;
  FontAwesome &operator=(const FontAwesome &) = delete;

  QFont m_font;
  bool m_initialized;
};