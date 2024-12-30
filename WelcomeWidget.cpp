#include "WelcomeWidget.h"
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QGuiApplication>
#include <QtGui/QStyleHints>
#include <QtGui/QIcon>
#include "ThemeManager.h"

WelcomeWidget::WelcomeWidget(QWidget *parent)
    : QWidget(parent), m_welcomeLabel(new QLabel("Welcome to WriteHand", this)), m_subtitleLabel(new QLabel("Your thoughts, beautifully organized", this)), m_newFileButton(new QPushButton(this))
{
  QVBoxLayout *layout = new QVBoxLayout(this);

  // Configure welcome label
  m_welcomeLabel->setAlignment(Qt::AlignCenter);

  // Configure subtitle label
  m_subtitleLabel->setAlignment(Qt::AlignCenter);

  // Configure new file button
  m_newFileButton->setIcon(QIcon::fromTheme("document-new"));
  m_newFileButton->setText("Create New File");
  m_newFileButton->setIconSize(QSize(24, 24));
  m_newFileButton->setFixedSize(200, 50);

  // Add widgets to layout with proper spacing
  layout->addStretch();
  layout->addWidget(m_welcomeLabel, 0, Qt::AlignCenter);
  layout->addWidget(m_subtitleLabel, 0, Qt::AlignCenter);
  layout->addSpacing(30);
  layout->addWidget(m_newFileButton, 0, Qt::AlignCenter);
  layout->addStretch();

  // Connect signals
  connect(m_newFileButton, &QPushButton::clicked, this, &WelcomeWidget::newFileRequested);
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &WelcomeWidget::updateTheme);

  // Set object names for styling
  m_welcomeLabel->setObjectName("welcomeLabel");
  m_subtitleLabel->setObjectName("subtitleLabel");

  updateTheme();
}

void WelcomeWidget::updateTheme()
{
  setStyleSheet(ThemeManager::instance().getStyleSheet("welcome"));
}