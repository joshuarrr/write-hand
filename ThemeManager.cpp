#include "ThemeManager.h"
#include <QGuiApplication>
#include <QStyleHints>
#include <QFile>

ThemeManager &ThemeManager::instance()
{
  static ThemeManager instance;
  return instance;
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
{
  initializeColors();
  loadStyleSheets();
  updateTheme();

  // Connect to system theme changes
  connect(qApp->styleHints(), &QStyleHints::colorSchemeChanged,
          this, &ThemeManager::updateTheme);
}

void ThemeManager::updateTheme()
{
  bool newDarkMode = QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
  if (m_isDarkMode != newDarkMode)
  {
    m_isDarkMode = newDarkMode;
    emit themeChanged(m_isDarkMode);
  }
}

bool ThemeManager::isDarkMode() const
{
  return m_isDarkMode;
}

QString ThemeManager::getStyleSheet(const QString &component) const
{
  const auto &styleSheets = m_isDarkMode ? m_darkStyleSheets : m_lightStyleSheets;
  return styleSheets.value(component);
}

QString ThemeManager::getColor(const QString &colorName) const
{
  const auto &colors = m_isDarkMode ? m_darkColors : m_lightColors;
  return colors.value(colorName);
}

void ThemeManager::initializeColors()
{
  // Dark theme colors
  m_darkColors = {
      {"background", "#2C2C2C"},
      {"text", "#E0E0E0"},
      {"accent", "#4682B4"},
      {"hover", "#3C3C3C"},
      {"selected", "#404040"},
      {"border", "#404040"},
      {"secondaryText", "#808080"}};

  // Light theme colors
  m_lightColors = {
      {"background", "#f5f5f5"},
      {"text", "#333333"},
      {"accent", "#4682B4"},
      {"hover", "#e0e0e0"},
      {"selected", "#d0d0d0"},
      {"border", "#d0d0d0"},
      {"secondaryText", "#666666"}};
}

void ThemeManager::loadStyleSheets()
{
  // Editor styles
  m_darkStyleSheets["editor"] = R"(
        QTextEdit {
            background-color: #2C2C2C;
            color: #E0E0E0;
            border: none;
            padding: 20px;
            selection-background-color: #4682B4;
            selection-color: white;
        }
    )";

  m_lightStyleSheets["editor"] = R"(
        QTextEdit {
            background-color: white;
            color: #333333;
            border: none;
            padding: 20px;
            selection-background-color: #4682B4;
            selection-color: white;
        }
    )";

  // FileTree styles
  m_darkStyleSheets["fileTree"] = R"(
        QTreeView {
            background-color: #2C2C2C;
            border: none;
            padding: 10px;
            font-size: 14px;
            color: #E0E0E0;
        }
        QTreeView::item {
            padding: 5px;
            border-radius: 3px;
        }
        QTreeView::item:hover {
            background-color: #3C3C3C;
        }
        QTreeView::item:selected {
            background-color: #4682B4;
            color: white;
        }
        QTreeView::branch {
            background-color: transparent;
            color: #E0E0E0;
        }
    )";

  m_lightStyleSheets["fileTree"] = R"(
        QTreeView {
            background-color: #f5f5f5;
            border: none;
            padding: 10px;
            font-size: 14px;
        }
        QTreeView::item {
            padding: 5px;
            border-radius: 3px;
        }
        QTreeView::item:hover {
            background-color: #e0e0e0;
        }
        QTreeView::item:selected {
            background-color: #4682B4;
            color: white;
        }
        QTreeView::branch {
            background-color: transparent;
        }
    )";

  // Toolbar styles
  m_darkStyleSheets["toolbar"] = R"(
        QToolBar {
            spacing: 8px;
            padding: 5px;
            background-color: #2C2C2C;
            border-bottom: 1px solid #404040;
        }
        QToolButton {
            border: none;
            border-radius: 5px;
            padding: 5px;
            min-width: 24px;
            color: #E0E0E0;
        }
        QToolButton:hover {
            background-color: #3C3C3C;
        }
        QToolButton:checked {
            background-color: #404040;
        }
        QToolBar::separator {
            width: 1px;
            background-color: #404040;
            margin: 4px 8px;
        }
    )";

  m_lightStyleSheets["toolbar"] = R"(
        QToolBar {
            spacing: 8px;
            padding: 5px;
            background-color: #f5f5f5;
            border-bottom: 1px solid #d0d0d0;
        }
        QToolButton {
            border: none;
            border-radius: 5px;
            padding: 5px;
            min-width: 24px;
        }
        QToolButton:hover {
            background-color: #e0e0e0;
        }
        QToolButton:checked {
            background-color: #d0d0d0;
        }
        QToolBar::separator {
            width: 1px;
            background-color: #d0d0d0;
            margin: 4px 8px;
        }
    )";

  // Welcome screen styles
  m_darkStyleSheets["welcome"] = R"(
        QWidget {
            background-color: #2C2C2C;
        }
        QLabel#welcomeLabel {
            color: #4682B4;
            font-size: 24px;
            font-weight: bold;
            margin-bottom: 10px;
        }
        QLabel#subtitleLabel {
            color: #808080;
            font-size: 16px;
            margin-bottom: 20px;
        }
        QPushButton {
            background-color: #4682B4;
            color: white;
            border-radius: 5px;
            font-size: 16px;
            padding: 10px;
            text-align: right;
            padding-right: 20px;
            border: none;
        }
        QPushButton:hover {
            background-color: #5B94C7;
        }
        QPushButton:pressed {
            background-color: #3A6B99;
        }
    )";

  m_lightStyleSheets["welcome"] = R"(
        QWidget {
            background-color: #f5f5f5;
        }
        QLabel#welcomeLabel {
            color: #4682B4;
            font-size: 24px;
            font-weight: bold;
            margin-bottom: 10px;
        }
        QLabel#subtitleLabel {
            color: #666666;
            font-size: 16px;
            margin-bottom: 20px;
        }
        QPushButton {
            background-color: #4682B4;
            color: white;
            border-radius: 5px;
            font-size: 16px;
            padding: 10px;
            text-align: right;
            padding-right: 20px;
            border: none;
        }
        QPushButton:hover {
            background-color: #5B94C7;
        }
        QPushButton:pressed {
            background-color: #3A6B99;
        }
    )";

  // Word count label styles
  m_darkStyleSheets["wordCount"] = R"(
        QLabel {
            background-color: #2C2C2C;
            color: #808080;
            border-top: 1px solid #404040;
        }
    )";

  m_lightStyleSheets["wordCount"] = R"(
        QLabel {
            background-color: white;
            color: #808080;
            border-top: 1px solid #d0d0d0;
        }
    )";

  // Splitter styles
  m_darkStyleSheets["splitter"] = R"(
        QSplitter::handle {
            width: 1px;
            background-color: #404040;
            margin: 2px;
        }
    )";

  m_lightStyleSheets["splitter"] = R"(
        QSplitter::handle {
            width: 1px;
            background-color: #d0d0d0;
            margin: 2px;
        }
    )";
}