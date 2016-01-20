#include "idleaircontroldialog.h"

IdleAirControlDialog::IdleAirControlDialog(QString title, QWidget* parent) :
  QDialog(parent)
{
  this->setWindowTitle(title + ": Idle Air Control Valve");

  m_iacGrid = new QGridLayout(this);

  m_stepCountBox = new QSpinBox(this);
  m_stepCountBox->setMinimum(0);
  m_stepCountBox->setMaximum(255);
  m_stepCountBox->setValue(255);

  m_stepCountLabel = new QLabel("Steps:", this);

  m_closeValveButton = new QRadioButton("Close valve", this);
  m_closeValveButton->setChecked(true);

  m_openValveButton = new QRadioButton("Open valve", this);

  m_sendCommandButton = new QPushButton("Send command", this);
  connect(m_sendCommandButton, SIGNAL(clicked()), this, SLOT(onSendCommand()));

  m_noteLabel = new QLabel(
    QString("Note: it is recommended to only test the idle air control movement\n") +
    QString(" when the engine is not running. Attempting to move the bypass valve\n") +
    QString(" while the engine is running may result in unpredictable behavior."), this);

  m_closeButton = new QPushButton("Close", this);
  connect(m_closeButton, SIGNAL(clicked()), this, SLOT(close()));

  //                                    row col rowSpan colSpan
  m_iacGrid->addWidget(m_closeValveButton,  0,  0,  1,      1,      Qt::AlignLeft);
  m_iacGrid->addWidget(m_openValveButton,   1,  0,  1,      1,      Qt::AlignLeft);
  m_iacGrid->addWidget(m_stepCountLabel,    0,  1,  2,      1,      Qt::AlignRight);
  m_iacGrid->addWidget(m_stepCountBox,      0,  2,  2,      1,      Qt::AlignLeft);
  m_iacGrid->addWidget(m_sendCommandButton, 0,  3,  2,      1,      Qt::AlignCenter);
  m_iacGrid->addWidget(m_noteLabel,         3,  0,  1,      4,      Qt::AlignLeft);
  m_iacGrid->addWidget(m_closeButton,       4,  3,  1,      1,      Qt::AlignRight);
}

void IdleAirControlDialog::onSendCommand()
{
  int direction = (m_openValveButton->isChecked()) ? 0 : 1;

  emit requestIdleAirControlMovement(direction, m_stepCountBox->value());
}
