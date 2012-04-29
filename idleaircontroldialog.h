#ifndef IDLEAIRCONTROLDIALOG_H
#define IDLEAIRCONTROLDIALOG_H

#include <QString>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QRadioButton>
#include <QPushButton>
#include "cuxinterface.h"

class IdleAirControlDialog : public QDialog
{
    Q_OBJECT
public:
    explicit IdleAirControlDialog(QString title, QWidget *parent = 0);

signals:
    void requestIdleAirControlMovement(int direction, int steps);

private:
    QGridLayout *iacGrid;
    QSpinBox *stepCountBox;
    QLabel *stepCountLabel;
    QRadioButton *closeValveButton;
    QRadioButton *openValveButton;
    QPushButton *sendCommandButton;
    QLabel *noteLabel;
    QPushButton *closeButton;

private slots:
    void onSendCommand();

};

#endif // IDLEAIRCONTROLDIALOG_H
