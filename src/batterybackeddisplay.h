#pragma once
#include <QDialog>
#include <QByteArray>
#include <QMap>
#include <QString>
#include <stdint.h>

namespace Ui
{
class BatteryBackedDisplay;
}

class BatteryBackedDisplay : public QDialog
{
  Q_OBJECT

public:
  BatteryBackedDisplay(QString title,
                       const QByteArray& batteryBackedMemory,
                       uint16_t startOffset,
                       const QMap<int,QString>& ramLabels,
                       QWidget* parent = 0);

private:
  Ui::BatteryBackedDisplay* m_ui;
};

