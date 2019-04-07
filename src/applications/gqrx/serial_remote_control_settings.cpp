#include "serial_remote_control_settings.h"

#include <iostream>

#include <QSerialPortInfo>

#include "ui_serial_remote_control_settings.h"

static const int DEFAULT_BAUD_RATE = 115200;

SerialRemoteControlSettings::SerialRemoteControlSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SerialRemoteControlSettings)
{
    ui->setupUi(this);

    detectPortNames();
    detectBaudRates();
}

SerialRemoteControlSettings::~SerialRemoteControlSettings()
{
    delete ui;
}

void SerialRemoteControlSettings::setPortName(QString portName)
{
    if (portName.isEmpty())
        return;

    int matchingIndex = ui->serialPortComboBox->findData(portName);

    if (matchingIndex == -1)
    {
        ui->serialPortComboBox->addItem(portName, portName);
        matchingIndex = ui->serialPortComboBox->count() - 1;
    }

    ui->serialPortComboBox->setCurrentIndex(matchingIndex);
}

QString SerialRemoteControlSettings::getPortName() const
{
    return ui->serialPortComboBox->currentData().toString();
}

void SerialRemoteControlSettings::setBaudRate(int baudRate)
{
    int matchingIndex = ui->baudRateComboBox->findData(baudRate);

    if (matchingIndex == -1)
    {
        ui->baudRateComboBox->addItem(QString::number(baudRate), baudRate);
        matchingIndex = ui->baudRateComboBox->count() - 1;
    }

    ui->baudRateComboBox->setCurrentIndex(matchingIndex);
}

int SerialRemoteControlSettings::getBaudRate() const
{
    bool ok = false;

    const int baudRate = ui->baudRateComboBox->currentData().toInt(&ok);
    if (!ok || baudRate == 0)
    {
      std::cout << "*** Invalid baud rate specified, replacing it with "
                << DEFAULT_BAUD_RATE << std::endl;
      return DEFAULT_BAUD_RATE;
    }

    return baudRate;
}

void SerialRemoteControlSettings::detectPortNames()
{
    ui->serialPortComboBox->clear();

    const QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    for (int i = 0; i < ports.size(); ++i)
    {
        const QSerialPortInfo& portInfo = ports[i];
        const QString portDesc = QString("%1 (%2)")
            .arg(portInfo.portName())
            .arg(portInfo.description());
        ui->serialPortComboBox->addItem(portDesc, portInfo.portName());
    }
}

void SerialRemoteControlSettings::detectBaudRates()
{
    ui->baudRateComboBox->clear();

    const QList<qint32> baudRates = QSerialPortInfo::standardBaudRates();

    for (int i = 0; i < baudRates.size(); ++i)
    {
        const int baudRate = baudRates[i];
        ui->baudRateComboBox->addItem(QString::number(baudRate), baudRate);
    }
}
