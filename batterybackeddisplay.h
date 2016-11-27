#ifndef BATTERYBACKEDDISPLAY_H
#define BATTERYBACKEDDISPLAY_H

#include <QDialog>
#include <QByteArray>

namespace Ui
{
class BatteryBackedDisplay;
}

class BatteryBackedDisplay : public QDialog
{
  Q_OBJECT

public:
  BatteryBackedDisplay(QString title, QByteArray* batteryBackedMemory, uint16_t startOffset, QWidget* parent = 0);

private slots:
  void refresh();

private:
  Ui::BatteryBackedDisplay* m_ui;
};

#endif // BATTERYBACKEDDISPLAY_H

