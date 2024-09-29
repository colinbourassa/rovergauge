#include "helpviewer.h"
#include <QFile>
#include <QTextDocument>
#include <QDesktopServices>

HelpViewer::HelpViewer(const QString title, QWidget* parent) :
  QDialog(parent)
{
  this->setWindowTitle(title + " - Help");
  this->setMinimumWidth(850);
  this->setMinimumHeight(550);

  m_vbox = new QVBoxLayout(this);
  m_closeButton = new QPushButton("Close", this);
  connect(m_closeButton, &QPushButton::clicked, this, &HelpViewer::onCloseClicked);
  m_viewer = new QTextBrowser(this);
  m_viewer->setOpenLinks(false);
  connect(m_viewer, &QTextBrowser::anchorClicked, this, &HelpViewer::onAnchorClicked);

  QFile helpFile(":/help/help.html");
  helpFile.open(QFile::ReadOnly);
  QString fileText = helpFile.readAll();
  helpFile.close();

  m_viewer->setHtml(fileText);

  m_vbox->addWidget(m_viewer);
  m_vbox->addWidget(m_closeButton);
}

void HelpViewer::onCloseClicked()
{
  this->hide();
}

void HelpViewer::onAnchorClicked(QUrl url)
{
  QDesktopServices::openUrl(url);
}

