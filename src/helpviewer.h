#pragma once
#include <QDialog>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QTextDocument>
#include <QUrl>

class HelpViewer : public QDialog
{
  Q_OBJECT
public:
  explicit HelpViewer(const QString title, QWidget* parent = nullptr);

private slots:
  void onCloseClicked();
  void onAnchorClicked(QUrl url);

private:
  QVBoxLayout* m_vbox = nullptr;
  QTextBrowser* m_viewer = nullptr;
  QPushButton* m_closeButton = nullptr;

};

