#include "ui_idleaircontroldialog.h"
#include "idleaircontroldialog.h"
#include "commonunits.h"

IdleAirControlDialog::IdleAirControlDialog(QString title, CUXInterface& cux, QWidget* parent) :
  m_ui(new Ui::IdleAirControlDialog),
  m_cux(cux),
  QDialog(parent)
{
  m_ui->setupUi(this);
  this->setWindowTitle(title + ": Idle Air Control Valve");

  connect(m_ui->m_sendCommandButton, &QPushButton::clicked, this, &IdleAirControlDialog::onSendCommand);
  connect(m_ui->m_closeButton, &QPushButton::clicked, this, &QDialog::close);
}

void IdleAirControlDialog::onSendCommand()
{
  const int direction = (m_ui->m_openValveButton->isChecked()) ? 1 : -1;
  const int steps = direction * m_ui->m_stepCountBox->value();

  m_cux.enqueueRequest(QueueableRequest_IACMotorDrive, steps);
}

