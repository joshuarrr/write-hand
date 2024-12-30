#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>

class ThemeManager : public QObject
{
  Q_OBJECT

public:
  static ThemeManager &instance();

  bool isDarkMode() const;
  QString getStyleSheet(const QString &component) const;
  QString getColor(const QString &colorName) const;

signals:
  void themeChanged(bool isDarkMode);

public slots:
  void updateTheme();

private:
  explicit ThemeManager(QObject *parent = nullptr);
  ~ThemeManager() = default;
  ThemeManager(const ThemeManager &) = delete;
  ThemeManager &operator=(const ThemeManager &) = delete;

  void loadStyleSheets();
  void initializeColors();

  bool m_isDarkMode;
  QMap<QString, QString> m_lightStyleSheets;
  QMap<QString, QString> m_darkStyleSheets;
  QMap<QString, QString> m_lightColors;
  QMap<QString, QString> m_darkColors;
};

#endif // THEMEMANAGER_H