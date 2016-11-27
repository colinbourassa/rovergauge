#include "ui_batterybackeddisplay.h"
#include "batterybackeddisplay.h"

/**
 * Constructor
 */
BatteryBackedDisplay::BatteryBackedDisplay(QString title, QByteArray *batteryBackedMemory, uint16_t startOffset, QWidget* parent) : QDialog(parent),
  m_ui(new Ui::BatteryBackedDisplay)
{
  m_ui->setupUi(this);
  this->setLayout(m_ui->m_mainLayout);

  this->setWindowTitle(title);

  QTableWidgetItem* item;

  for (unsigned int idx = 0; idx < batteryBackedMemory->size(); ++idx)
  {
    m_ui->m_batteryBackedTable->insertRow(idx);

    item = new QTableWidgetItem(QString("%1").arg(startOffset + idx, 0, 16).toUpper());
    m_ui->m_batteryBackedTable->setItem(idx, 0, item);
    item = new QTableWidgetItem(QString("%1").arg(batteryBackedMemory->at(idx), 0, 16).toUpper());
    m_ui->m_batteryBackedTable->setItem(idx, 1, item);
  }
}

void BatteryBackedDisplay::refresh()
{
  //TODO
}
