#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QFrame>

class EditorWidget : public QWidget
{
  Q_OBJECT

public:
  EditorWidget(QWidget *parent = nullptr);
  void setContent(const QString &content, bool isRichText = false);
  QString content(bool asRichText = false) const;
  void clear();
  QTextEdit *editor() const { return m_editor; }

signals:
  void contentChanged();

public slots:
  void showFindReplace();
  void hideFindReplace();
  void findNext();
  void findPrevious();
  void replace();
  void replaceAll();
  void updateSearch();

private:
  void setupFindReplaceWidget();
  bool findText(const QString &text, QTextDocument::FindFlags flags = {});
  void clearHighlights();

  QTextEdit *m_editor;
  QFrame *m_findReplaceWidget;
  QLineEdit *m_findLineEdit;
  QLineEdit *m_replaceLineEdit;
  QPushButton *m_findPrevButton;
  QPushButton *m_findNextButton;
  QPushButton *m_replaceButton;
  QPushButton *m_replaceAllButton;
  QPushButton *m_closeButton;
  QLabel *m_matchLabel;
  int m_currentMatch;
  int m_totalMatches;
};