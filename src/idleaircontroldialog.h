#pragma once
#include <QString>
#include <QDialog>
#include "cuxinterface.h"

namespace Ui
{
class IdleAirControlDialog;
}

class IdleAirControlDialog : public QDialog
{
  Q_OBJECT
public:
  explicit IdleAirControlDialog(QString title, CUXInterface& cux, QWidget* parent = 0);

private:
  Ui::IdleAirControlDialog* m_ui;
  CUXInterface& m_cux;

private slots:
  void onSendCommand();

};

