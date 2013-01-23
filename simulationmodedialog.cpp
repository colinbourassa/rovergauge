#include <QMessageBox>
#include "simulationmodedialog.h"

SimulationModeDialog::SimulationModeDialog(const QString title, QWidget *parent) :
    QDialog(parent)
{
    this->setWindowTitle(title);
    setupWidgets();
}

void SimulationModeDialog::setupWidgets()
{
    grid = new QGridLayout(this);

    inertiaSwitchLabel = new QLabel("Inertia switch:", this);
    heatedScreenLabel = new QLabel("Heated screen:", this);
    airflowALabel = new QLabel("Airflow A:", this);
    airflowBLabel = new QLabel("Airflow B:", this);
    airflowCLabel = new QLabel("Airflow C:", this);
    throttlePositionLabel = new QLabel("Throttle position:", this);
    coolantTempLabel = new QLabel("Coolant temperature:", this);
    fuelTempLabel = new QLabel("Fuel temperature:", this);
    neutralSwitchLabel = new QLabel("Neutral switch:", this);
    airConLoadLabel = new QLabel("Air conditioner load:", this);
    roadSpeedLabel = new QLabel("Road speed:", this);
    mainRelayLabel = new QLabel("Main relay voltage:", this);
    tuneResistorLabel = new QLabel("Tune resistor:", this);
    o2SensorReferenceLabel = new QLabel("O2 sensor reference:", this);
    diagnosticPlugLabel = new QLabel("Diagnostic plug:", this);

    inertiaSwitchVal = new QLineEdit(this);
    heatedScreenVal = new QLineEdit(this);
    airflowAVal = new QLineEdit(this);
    airflowBVal = new QLineEdit(this);
    airflowCVal = new QLineEdit(this);
    throttlePositionVal = new QLineEdit(this);
    coolantTempVal = new QLineEdit(this);
    fuelTempVal = new QLineEdit(this);
    neutralSwitchVal = new QLineEdit(this);
    airConLoadVal = new QLineEdit(this);
    roadSpeedVal = new QLineEdit(this);
    mainRelayVal = new QLineEdit(this);
    tuneResistorVal = new QLineEdit(this);
    o2SensorReferenceVal = new QLineEdit(this);
    diagnosticPlugVal = new QLineEdit(this);

    inertiaSwitchBox = new QCheckBox(this);
    neutralSwitchBox = new QCheckBox(this);
    heatedScreenBox = new QCheckBox(this);
    coolantTempSlider = new QSlider(Qt::Horizontal, this);
    coolantTempSlider->setMinimum(-40);
    coolantTempSlider->setMaximum(250);
    fuelTempSlider = new QSlider(Qt::Horizontal, this);
    fuelTempSlider->setMinimum(-40);
    fuelTempSlider->setMaximum(250);

    writeButton = new QPushButton("Write", this);
    connect(writeButton, SIGNAL(clicked()), this, SLOT(onWriteClicked()));
    closeButton = new QPushButton("Close", this);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(onCloseClicked()));

    int row = 0;

    grid->addWidget(inertiaSwitchLabel,     row,   0, Qt::AlignRight);
    grid->addWidget(inertiaSwitchBox,       row,   1, Qt::AlignCenter);
    grid->addWidget(inertiaSwitchVal,       row++, 2, Qt::AlignLeft);

    grid->addWidget(heatedScreenLabel,      row,   0, Qt::AlignRight);
    grid->addWidget(heatedScreenBox,        row,   1, Qt::AlignCenter);
    grid->addWidget(heatedScreenVal,        row++, 2, Qt::AlignLeft);

    grid->addWidget(airflowALabel,          row,   0, Qt::AlignRight);
    grid->addWidget(airflowAVal,            row++, 2, Qt::AlignLeft);

    grid->addWidget(airflowBLabel,          row,   0, Qt::AlignRight);
    grid->addWidget(airflowBVal,            row++, 2, Qt::AlignLeft);

    grid->addWidget(airflowCLabel,          row,   0, Qt::AlignRight);
    grid->addWidget(airflowCVal,            row++, 2, Qt::AlignLeft);

    grid->addWidget(throttlePositionLabel,  row,   0, Qt::AlignRight);
    grid->addWidget(throttlePositionVal,    row++, 2, Qt::AlignLeft);

    grid->addWidget(coolantTempLabel,       row,   0, Qt::AlignRight);
    grid->addWidget(coolantTempSlider,      row,   1, Qt::AlignCenter);
    grid->addWidget(coolantTempVal,         row++, 2, Qt::AlignLeft);

    grid->addWidget(fuelTempLabel,          row,   0, Qt::AlignRight);
    grid->addWidget(fuelTempSlider,         row,   1, Qt::AlignCenter);
    grid->addWidget(fuelTempVal,            row++, 2, Qt::AlignLeft);

    grid->addWidget(neutralSwitchLabel,     row,   0, Qt::AlignRight);
    grid->addWidget(neutralSwitchBox,       row,   1, Qt::AlignCenter);
    grid->addWidget(neutralSwitchVal,       row++, 2, Qt::AlignLeft);

    grid->addWidget(airConLoadLabel,        row,   0, Qt::AlignRight);
    grid->addWidget(airConLoadVal,          row++, 2, Qt::AlignLeft);

    grid->addWidget(roadSpeedLabel,         row,   0, Qt::AlignRight);
    grid->addWidget(roadSpeedVal,           row++, 2, Qt::AlignLeft);

    grid->addWidget(mainRelayLabel,         row,   0, Qt::AlignRight);
    grid->addWidget(mainRelayVal,           row++, 2, Qt::AlignLeft);

    grid->addWidget(tuneResistorLabel,      row,   0, Qt::AlignRight);
    grid->addWidget(tuneResistorVal,        row++, 2, Qt::AlignLeft);

    grid->addWidget(o2SensorReferenceLabel, row,   0, Qt::AlignRight);
    grid->addWidget(o2SensorReferenceVal,   row++, 2, Qt::AlignLeft);

    grid->addWidget(diagnosticPlugLabel,    row,   0, Qt::AlignRight);
    grid->addWidget(diagnosticPlugVal,      row++, 2, Qt::AlignLeft);

    grid->addWidget(writeButton, row, 1);
    grid->addWidget(closeButton, row, 2);
}

void SimulationModeDialog::onCloseClicked()
{
    this->hide();
}

void SimulationModeDialog::onWriteClicked()
{
    QMessageBox::information(this, "Placeholder", "Writing!", QMessageBox::Ok);
}
