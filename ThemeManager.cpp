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

QString ThemeManager::getStyleSheet(const QString &widget) const
{
  if (widget == "columnView")
  {
    return QString(R"(
            QColumnView {
                background-color: #171717;
                border: none;
                min-width: 200px;
            }
            QColumnView::item {
                padding: 6px 8px;
                border: none;
            }
            QColumnView::item:selected {
                background-color: #2D2D2D;
            }
            QColumnView::item:hover {
                background-color: #252525;
            }
            QAbstractScrollArea {
                background-color: #171717;
                border: none;
            }
            QListView {
                background-color: #171717;
                border-right: 1px solid #252525;
                min-width: 200px;
                max-width: 250px;
                padding: 0;
                margin: 0;
            }
            QScrollBar:horizontal {
                height: 8px;
                background: #171717;
            }
            QScrollBar::handle:horizontal {
                background: #404040;
                min-width: 20px;
                border-radius: 4px;
            }
            QScrollBar::add-line:horizontal,
            QScrollBar::sub-line:horizontal {
                width: 0;
                height: 0;
            }
            QScrollBar:vertical {
                width: 8px;
                background: #171717;
            }
            QScrollBar::handle:vertical {
                background: #404040;
                min-height: 20px;
                border-radius: 4px;
            }
            QScrollBar::add-line:vertical,
            QScrollBar::sub-line:vertical {
                width: 0;
                height: 0;
            }
        )");
  }
  const auto &styleSheets = m_isDarkMode ? m_darkStyleSheets : m_lightStyleSheets;
  return styleSheets.value(widget);
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
      {"background", "#171717"},
      {"text", "#E0E0E0"},
      {"accent", "#4682B4"},
      {"hover", "#252525"},
      {"selected", "#282828"},
      {"border", "#282828"},
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
            background-color: #171717;
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
            background-color: #171717;
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
            background-color: #252525;
        }
        QTreeView::item:selected {
            background-color: #282828;
            color: #E0E0E0;
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
            background-color: #171717;
            border-bottom: 1px solid #282828;
        }
        QToolButton {
            border: none;
            border-radius: 5px;
            padding: 5px;
            min-width: 24px;
            color: #E0E0E0;
            background: transparent;
        }
        QToolButton:hover {
            background-color: #252525;
        }
        QToolButton:checked {
            background-color: transparent;
        }
        QToolButton#sidebarButton {
            background-color: transparent;
        }
        QToolButton#sidebarButton:hover {
            background-color: transparent;
        }
        QToolButton#sidebarButton:checked {
            background-color: transparent;
        }
        QToolBar::separator {
            width: 1px;
            background-color: #282828;
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
            background-color: #171717;
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
            background-color: #171717;
            color: #808080;
            border-top: 1px solid #282828;
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
            background-color: #282828;
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

  // Column view styles
  m_darkStyleSheets["columnView"] = R"(
        QColumnView {
            background-color: #171717;
            border: none;
            color: #E0E0E0;
            font-size: 13px;
        }
        QColumnView::item {
            padding: 6px 8px;
            border: none;
        }
        QColumnView::item:hover {
            background-color: #252525;
        }
        QColumnView::item:selected {
            background-color: #282828;
            color: #E0E0E0;
        }
        QListView {
            background-color: #171717;
            border-right: 1px solid #282828;
            font-size: 13px;
            min-width: 180px;
            padding: 4px;
            margin: 0;
            spacing: 0;
        }
        QListView::item {
            padding: 6px 8px;
            border: none;
            margin: 0;
        }
        QListView::item:hover {
            background-color: #252525;
        }
        QListView::item:selected {
            background-color: #282828;
            color: #E0E0E0;
        }
        QScrollBar:vertical {
            border: none;
            background: #171717;
            width: 12px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background: #404040;
            min-height: 20px;
            border-radius: 6px;
            margin: 2px;
        }
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0;
            border: none;
            background: none;
        }
        QScrollBar::add-page:vertical,
        QScrollBar::sub-page:vertical {
            background: none;
        }
    )";

  m_lightStyleSheets["columnView"] = R"(
        QColumnView {
            background-color: #f5f5f5;
            border: none;
            color: #333333;
        }
        QColumnView::item {
            padding: 8px;
            border: none;
        }
        QColumnView::item:hover {
            background-color: #e0e0e0;
        }
        QColumnView::item:selected {
            background-color: #4682B4;
            color: white;
        }
        QListView {
            background-color: #f5f5f5;
            border-right: 1px solid #d0d0d0;
            font-size: 14px;
            padding: 4px;
        }
        QListView::item {
            padding: 8px;
            border-radius: 4px;
            margin: 2px;
        }
        QListView::item:hover {
            background-color: #e0e0e0;
        }
        QListView::item:selected {
            background-color: #4682B4;
            color: white;
        }
    )";
}