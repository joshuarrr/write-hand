#ifndef WELCOMEWIDGET_H
#define WELCOMEWIDGET_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

class WelcomeWidget : public QWidget
{
  Q_OBJECT

public:
  explicit WelcomeWidget(QWidget *parent = nullptr);

  void updateTheme();

signals:
  void newFileRequested();

private:
  QLabel *m_welcomeLabel;
  QLabel *m_subtitleLabel;
  QPushButton *m_newFileButton;
};

#endif // WELCOMEWIDGET_H