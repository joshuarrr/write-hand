#include "EditorWidget.h"
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QGuiApplication>
#include <QtGui/QStyleHints>
#include <QtGui/QFont>
#include <QtCore/QRegularExpression>
#include "ThemeManager.h"
#include <QtGui/QTextCharFormat>
#include <QtGui/QTextDocument>
#include <QtGui/QTextCursor>
#include <QtCore/QDebug>

EditorWidget::EditorWidget(QWidget *parent)
    : QWidget(parent), m_editor(new QTextEdit(this)), m_currentMatch(0), m_totalMatches(0)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  // Create and hide find/replace widget
  m_findReplaceWidget = new QFrame(this);
  m_findReplaceWidget->setFrameStyle(QFrame::StyledPanel);
  m_findReplaceWidget->hide();
  setupFindReplaceWidget();
  layout->addWidget(m_findReplaceWidget);

  // Add editor
  layout->addWidget(m_editor);

  // Connect editor signals
  connect(m_editor, &QTextEdit::textChanged, this, [this]()
          {
        emit contentChanged();
        if (m_findReplaceWidget->isVisible()) {
            updateSearch();
        } });

  // Set up editor properties
  m_editor->setAcceptRichText(true);
  m_editor->setLineWrapMode(QTextEdit::WidgetWidth);
  QFont font = m_editor->font();
  font.setPointSize(14);
  m_editor->setFont(font);

  // Ensure editor uses our highlight colors
  QPalette p = m_editor->palette();
  p.setColor(QPalette::Highlight, QColor(255, 255, 0));
  p.setColor(QPalette::HighlightedText, Qt::black);
  m_editor->setPalette(p);
}

void EditorWidget::setupFindReplaceWidget()
{
  QVBoxLayout *mainLayout = new QVBoxLayout(m_findReplaceWidget);
  mainLayout->setContentsMargins(8, 8, 8, 8);
  mainLayout->setSpacing(8);

  // Find row
  QHBoxLayout *findLayout = new QHBoxLayout();
  m_findLineEdit = new QLineEdit(m_findReplaceWidget);
  m_findLineEdit->setPlaceholderText("Find");
  connect(m_findLineEdit, &QLineEdit::textChanged, this, &EditorWidget::updateSearch);
  connect(m_findLineEdit, &QLineEdit::returnPressed, this, &EditorWidget::findNext);

  m_findPrevButton = new QPushButton("Previous", m_findReplaceWidget);
  m_findNextButton = new QPushButton("Next", m_findReplaceWidget);
  m_closeButton = new QPushButton("×", m_findReplaceWidget);
  m_closeButton->setFixedSize(24, 24);

  connect(m_findPrevButton, &QPushButton::clicked, this, &EditorWidget::findPrevious);
  connect(m_findNextButton, &QPushButton::clicked, this, &EditorWidget::findNext);
  connect(m_closeButton, &QPushButton::clicked, this, &EditorWidget::hideFindReplace);

  findLayout->addWidget(m_findLineEdit);
  findLayout->addWidget(m_findPrevButton);
  findLayout->addWidget(m_findNextButton);
  findLayout->addWidget(m_closeButton);

  // Replace row
  QHBoxLayout *replaceLayout = new QHBoxLayout();
  m_replaceLineEdit = new QLineEdit(m_findReplaceWidget);
  m_replaceLineEdit->setPlaceholderText("Replace with");
  connect(m_replaceLineEdit, &QLineEdit::returnPressed, this, &EditorWidget::replace);

  m_replaceButton = new QPushButton("Replace", m_findReplaceWidget);
  m_replaceAllButton = new QPushButton("Replace All", m_findReplaceWidget);

  connect(m_replaceButton, &QPushButton::clicked, this, &EditorWidget::replace);
  connect(m_replaceAllButton, &QPushButton::clicked, this, &EditorWidget::replaceAll);

  replaceLayout->addWidget(m_replaceLineEdit);
  replaceLayout->addWidget(m_replaceButton);
  replaceLayout->addWidget(m_replaceAllButton);
  replaceLayout->addSpacing(24); // Align with close button

  // Match count label
  m_matchLabel = new QLabel(m_findReplaceWidget);
  m_matchLabel->setAlignment(Qt::AlignRight);

  mainLayout->addLayout(findLayout);
  mainLayout->addLayout(replaceLayout);
  mainLayout->addWidget(m_matchLabel);
}

void EditorWidget::showFindReplace()
{
  m_findReplaceWidget->show();
  m_findLineEdit->setFocus();
  m_findLineEdit->selectAll();
  updateSearch();
}

void EditorWidget::hideFindReplace()
{
  clearHighlights();
  m_findLineEdit->clear();
  m_findReplaceWidget->hide();
  m_editor->setFocus();
}

// TODO: Fix the partial highlight issue where the first selected match shows both
// highlight and underline styles competing. Current attempts included:
// 1. Applying underlines first, then letting selection override
// 2. Using light yellow background instead of underlines
// 3. Managing our own highlight/selection system
// None fully resolved the issue - needs further investigation.
void EditorWidget::updateSearch()
{
  static bool isUpdating = false;
  if (isUpdating)
    return;
  isUpdating = true;

  QString searchText = m_findLineEdit->text();
  if (searchText.isEmpty())
  {
    m_editor->setExtraSelections(QList<QTextEdit::ExtraSelection>());
    m_matchLabel->clear();
    m_currentMatch = 0;
    m_totalMatches = 0;
    isUpdating = false;
    return;
  }

  // First pass: count matches
  QTextCursor cursor = QTextCursor(m_editor->document());
  m_totalMatches = 0;
  cursor.setPosition(0);
  QTextCursor firstMatch;

  while (!cursor.isNull() && !cursor.atEnd())
  {
    cursor = m_editor->document()->find(searchText, cursor);
    if (!cursor.isNull())
    {
      if (m_totalMatches == 0)
      {
        firstMatch = cursor;
      }
      m_totalMatches++;
    }
  }

  if (m_totalMatches == 0)
  {
    m_editor->setExtraSelections(QList<QTextEdit::ExtraSelection>());
    m_matchLabel->setText("No matches");
    isUpdating = false;
    return;
  }

  // Get current selection or use first match
  QTextCursor currentCursor = m_editor->textCursor();
  bool hasSelection = currentCursor.hasSelection();

  if (!hasSelection && m_totalMatches > 0)
  {
    currentCursor = firstMatch;
    m_currentMatch = 0;
  }

  // Apply formatting to all matches
  QList<QTextEdit::ExtraSelection> extraSelections;
  cursor = QTextCursor(m_editor->document());
  cursor.setPosition(0);

  while (!cursor.isNull() && !cursor.atEnd())
  {
    cursor = m_editor->document()->find(searchText, cursor);
    if (!cursor.isNull())
    {
      QTextEdit::ExtraSelection selection;
      selection.cursor = cursor;
      QTextCharFormat format;

      // If this is the current match
      if (cursor.selectionStart() == currentCursor.selectionStart() &&
          cursor.selectionEnd() == currentCursor.selectionEnd())
      {
        format.setBackground(m_editor->palette().color(QPalette::Highlight));
        format.setForeground(m_editor->palette().color(QPalette::HighlightedText));
      }
      else
      {
        format.setBackground(Qt::transparent);
        format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        format.setUnderlineColor(QColor(255, 255, 0));
      }

      selection.format = format;
      extraSelections.append(selection);
    }
  }

  // Update match label
  m_matchLabel->setText(QString("%1 of %2 matches").arg(m_currentMatch + 1).arg(m_totalMatches));

  // Apply all formatting at once
  m_editor->setExtraSelections(extraSelections);

  // Set cursor position without selection to avoid Qt's native highlighting
  if (!hasSelection)
  {
    m_editor->setTextCursor(currentCursor);
  }

  isUpdating = false;
}

bool EditorWidget::findText(const QString &text, QTextDocument::FindFlags flags)
{
  if (text.isEmpty())
    return false;

  bool found = m_editor->find(text, flags);

  if (found)
  {
    // Update current match number
    QTextCursor cursor = m_editor->textCursor();
    int position = cursor.position();
    cursor.setPosition(0);
    m_currentMatch = 0;
    while (cursor.position() < position)
    {
      cursor = m_editor->document()->find(text, cursor);
      if (cursor.isNull())
        break;
      m_currentMatch++;
    }

    // Only update the underlines, don't reset the selection
    static bool isUpdating = false;
    if (!isUpdating)
    {
      isUpdating = true;
      updateSearch();
      isUpdating = false;
    }
  }

  return found;
}

void EditorWidget::findNext()
{
  QString text = m_findLineEdit->text();
  if (!findText(text))
  {
    // If not found, wrap around to the beginning
    QTextCursor cursor = m_editor->textCursor();
    cursor.movePosition(QTextCursor::Start);
    m_editor->setTextCursor(cursor);
    findText(text);
  }
}

void EditorWidget::findPrevious()
{
  QString text = m_findLineEdit->text();
  if (!findText(text, QTextDocument::FindBackward))
  {
    // If not found, wrap around to the end
    QTextCursor cursor = m_editor->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_editor->setTextCursor(cursor);
    findText(text, QTextDocument::FindBackward);
  }
}

void EditorWidget::replace()
{
  QString findText = m_findLineEdit->text();
  QString replaceText = m_replaceLineEdit->text();

  if (findText.isEmpty())
    return;

  QTextCursor cursor = m_editor->textCursor();
  if (cursor.hasSelection())
  {
    QString selectedText = cursor.selectedText();
    if (selectedText == findText)
    {
      cursor.insertText(replaceText);
      updateSearch();
    }
    findNext();
  }
  else
  {
    findNext();
  }
}

void EditorWidget::replaceAll()
{
  QString findText = m_findLineEdit->text();
  QString replaceText = m_replaceLineEdit->text();

  if (findText.isEmpty())
    return;

  QTextCursor cursor = m_editor->textCursor();
  cursor.beginEditBlock();

  cursor.movePosition(QTextCursor::Start);
  m_editor->setTextCursor(cursor);

  int replacements = 0;
  QTextDocument::FindFlags flags = QTextDocument::FindWholeWords;
  while (m_editor->find(findText, flags))
  {
    cursor = m_editor->textCursor();
    cursor.insertText(replaceText);
    replacements++;
  }

  cursor.endEditBlock();

  m_matchLabel->setText(QString("%1 replacements made").arg(replacements));
  updateSearch();
}

void EditorWidget::setContent(const QString &content, bool isRichText)
{
  if (isRichText)
  {
    m_editor->setHtml(content);
  }
  else
  {
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

void EditorWidget::clearHighlights()
{
  m_editor->setExtraSelections(QList<QTextEdit::ExtraSelection>());
}