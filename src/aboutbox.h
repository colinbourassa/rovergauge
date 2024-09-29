#pragma once
#include <QDialog>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QStyle>
#include <QString>
#include "comm14cux.h"

/**
 * The "About" dialog that shows information about the program.
 */
class AboutBox : public QDialog
{
  Q_OBJECT

public:
  AboutBox(QStyle* parentStyle, QString title, c14cux_version version, QWidget* parent);

private:
  void setupWidgets();
  QString makeVersionString(int maj, int min, int patch);

  QStyle* m_style;
  QGridLayout* m_grid = nullptr;

  c14cux_version m_ver;

  const QString m_urlString;
  const QString m_urlLibString;
  const QString m_aboutString;
  QLabel* m_iconLabel = nullptr;
  QLabel* m_info = nullptr;
  QLabel* m_name = nullptr;
  QLabel* m_infoLib = nullptr;
  QLabel* m_url = nullptr;
  QLabel* m_urlLib = nullptr;
  QPushButton* m_ok = nullptr;
};

