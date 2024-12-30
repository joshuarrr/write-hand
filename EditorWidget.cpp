#include "EditorWidget.h"
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QGuiApplication>
#include <QtGui/QStyleHints>
#include <QtGui/QFont>
#include <QtCore/QRegularExpression>
#include "ThemeManager.h"

EditorWidget::EditorWidget(QWidget *parent)
    : QWidget(parent), m_editor(new QTextEdit(this)), m_wordCountLabel(new QLabel(this))
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  setupEditor();
  updateTheme();

  layout->addWidget(m_editor);
  layout->addWidget(m_wordCountLabel);

  connect(m_editor->document(), &QTextDocument::contentsChanged, this, &EditorWidget::contentChanged);
  connect(m_editor->document(), &QTextDocument::contentsChanged, this, &EditorWidget::updateWordCount);
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &EditorWidget::updateTheme);
}

void EditorWidget::setupEditor()
{
  // Set a nice default font
  QFont font("SF Pro", 14);
  font.setStyleHint(QFont::SansSerif);
  m_editor->setFont(font);

  // Set document margins for better readability
  m_editor->document()->setDocumentMargin(20);

  // Set line spacing
  QTextBlockFormat format;
  format.setLineHeight(150, QTextBlockFormat::ProportionalHeight);

  QTextCursor cursor = m_editor->textCursor();
  cursor.select(QTextCursor::Document);
  cursor.mergeBlockFormat(format);
  cursor.clearSelection();
  m_editor->setTextCursor(cursor);

  // Set tab stop width
  m_editor->setTabStopDistance(QFontMetricsF(m_editor->font()).horizontalAdvance(' ') * 4);

  // Style the word count label
  m_wordCountLabel->setAlignment(Qt::AlignRight);
  m_wordCountLabel->setContentsMargins(10, 5, 10, 5);
  updateWordCount();
}

void EditorWidget::updateTheme()
{
  auto &theme = ThemeManager::instance();
  m_editor->setStyleSheet(theme.getStyleSheet("editor"));
  m_wordCountLabel->setStyleSheet(theme.getStyleSheet("wordCount"));
}

void EditorWidget::setContent(const QString &content, bool isRichText)
{
  if (isRichText)
  {
    m_editor->setAcceptRichText(true);
    m_editor->setHtml(content);
  }
  else
  {
    m_editor->setAcceptRichText(false);
    m_editor->setPlainText(content);
  }
}

QString EditorWidget::content(bool asRichText) const
{
  return asRichText ? m_editor->toHtml() : m_editor->toPlainText();
}

void EditorWidget::clear()
{
  m_editor->clear();
}

void EditorWidget::updateWordCount()
{
  QString text = m_editor->toPlainText();
  int words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).count();
  int chars = text.length();
  m_wordCountLabel->setText(QString("%1 words, %2 characters").arg(words).arg(chars));
}