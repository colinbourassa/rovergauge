#include "ui_batterybackeddisplay.h"
#include "batterybackeddisplay.h"

BatteryBackedDisplay::BatteryBackedDisplay(const QString& title,
                                           const QByteArray& batteryBackedMemory,
                                           uint16_t startOffset,
                                           const QMap<int,QString>& ramOffsetLabels,
                                           QWidget* parent) :
  QDialog(parent),
  m_ui(new Ui::BatteryBackedDisplay)
{
  m_ui->setupUi(this);
  this->setWindowTitle(title);
  connect(m_ui->m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
  QTableWidgetItem* item;
  uint8_t currentByte = 0;

  for (unsigned int idx = 0; idx < batteryBackedMemory.size(); ++idx)
  {
    m_ui->m_batteryBackedTable->insertRow(idx);

    const uint16_t addr = startOffset + idx;
    item = new QTableWidgetItem(QString("%1").arg(addr, 0, 16).toUpper());
    m_ui->m_batteryBackedTable->setItem(idx, 0, item);

    currentByte = (uint8_t)batteryBackedMemory.at(idx);
    item = new QTableWidgetItem(QString("%1").arg(currentByte, 0, 16).toUpper());
    m_ui->m_batteryBackedTable->setItem(idx, 1, item);

    if (ramOffsetLabels.count(addr))
    {
      item = new QTableWidgetItem(ramOffsetLabels[addr]);
      m_ui->m_batteryBackedTable->setItem(idx, 2, item);
    }
  }

  m_ui->m_batteryBackedTable->resizeColumnsToContents();
  m_ui->m_batteryBackedTable->resizeRowsToContents();
}

