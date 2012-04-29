#include "idleaircontroldialog.h"

IdleAirControlDialog::IdleAirControlDialog(QString title, QWidget *parent) :
    QDialog(parent)
{
    this->setWindowTitle(title + ": Idle Air Control Valve");

    iacGrid = new QGridLayout(this);

    stepCountBox = new QSpinBox(this);
    stepCountBox->setMinimum(0);
    stepCountBox->setMaximum(255);
    stepCountBox->setValue(255);

    stepCountLabel = new QLabel("Steps:", this);

    closeValveButton = new QRadioButton("Close valve", this);
    closeValveButton->setChecked(true);

    openValveButton = new QRadioButton("Open valve", this);

    sendCommandButton = new QPushButton("Send command", this);
    connect(sendCommandButton, SIGNAL(clicked()), this, SLOT(onSendCommand()));

    noteLabel = new QLabel(
                QString("Note: it is recommended to only test the idle air control movement\n") +
                QString(" when the engine is not running. Attempting to move the bypass valve\n") +
                QString(" while the engine is running may result in unpredictable behavior."), this);

    closeButton = new QPushButton("Close", this);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    //                                    row col rowSpan colSpan
    iacGrid->addWidget(closeValveButton,  0,  0,  1,      1,      Qt::AlignLeft);
    iacGrid->addWidget(openValveButton,   1,  0,  1,      1,      Qt::AlignLeft);
    iacGrid->addWidget(stepCountLabel,    0,  1,  2,      1,      Qt::AlignRight);
    iacGrid->addWidget(stepCountBox,      0,  2,  2,      1,      Qt::AlignLeft);
    iacGrid->addWidget(sendCommandButton, 0,  3,  2,      1,      Qt::AlignCenter);
    iacGrid->addWidget(noteLabel,         3,  0,  1,      4,      Qt::AlignLeft);
    iacGrid->addWidget(closeButton,       4,  3,  1,      1,      Qt::AlignRight);
}

void IdleAirControlDialog::onSendCommand()
{
    int direction = (openValveButton->isChecked()) ? 0 : 1;

    emit requestIdleAirControlMovement(direction, stepCountBox->value());
}
