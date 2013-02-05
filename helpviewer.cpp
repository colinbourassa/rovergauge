#include "helpviewer.h"
#include <QFile>
#include <QTextDocument>

HelpViewer::HelpViewer(const QString title, QWidget *parent) :
    QDialog(parent),
    m_helpDoc(0),
    m_vbox(0),
    m_viewer(0),
    m_closeButton(0)
{
    this->setWindowTitle(title + " - Help");
    this->setMinimumWidth(850);
    this->setMinimumHeight(550);

    m_vbox = new QVBoxLayout(this);
    m_closeButton = new QPushButton("Close", this);
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(onCloseClicked()));
    m_viewer = new QTextBrowser(this);
    m_helpDoc = new QTextDocument(this);

    QFile helpFile(":/help/help.html");
    helpFile.open(QFile::ReadOnly);
    QString fileText = helpFile.readAll();
    helpFile.close();

    m_helpDoc->setHtml(fileText);

    m_viewer->setDocument(m_helpDoc);
    m_vbox->addWidget(m_viewer);
    m_vbox->addWidget(m_closeButton);
}

void HelpViewer::onCloseClicked()
{
    this->hide();
}
