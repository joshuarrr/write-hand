#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QLabel>

class EditorWidget : public QWidget
{
  Q_OBJECT

public:
  explicit EditorWidget(QWidget *parent = nullptr);

  void setContent(const QString &content, bool isRichText);
  QString content(bool asRichText) const;
  void clear();
  void updateTheme();
  QTextEdit *editor() const { return m_editor; }

signals:
  void contentChanged();

private:
  void setupEditor();
  void updateWordCount();

  QTextEdit *m_editor;
  QLabel *m_wordCountLabel;
};

#endif // EDITORWIDGET_H