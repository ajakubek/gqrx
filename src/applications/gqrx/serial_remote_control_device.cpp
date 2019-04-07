/* -*- c++ -*- */
/*
 * Gqrx SDR: Software defined radio receiver powered by GNU Radio and Qt
 *           http://gqrx.dk/
 *
 * Copyright 2013 Alexandru Csete OZ9AEC.
 * Copyright 2019 Adam Jakubek
 *
 * Gqrx is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * Gqrx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gqrx; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#include "serial_remote_control_device.h"

#include <iostream>

#include <QSerialPort>
#include <QSettings>

#include "remote_control.h"

static const char* SETTINGS_SECTION_NAME = "serial_remote_control_device";
static const char* DEFAULT_PORT_NAME = "";
static const int DEFAULT_BAUD_RATE = 115200;

SerialRemoteControlDevice::SerialRemoteControlDevice(RemoteControl *remote_control,
                                                     QObject *parent)
    : QObject(parent), remote_control(remote_control), rc_serial_port(0)
{
    setPortName(DEFAULT_PORT_NAME);
    setBaudRate(DEFAULT_BAUD_RATE);
}

SerialRemoteControlDevice::~SerialRemoteControlDevice()
{
    closeDevice();
}

bool SerialRemoteControlDevice::isDeviceOpen() const
{
    return rc_serial_port != 0 && rc_serial_port->isOpen();
}

bool SerialRemoteControlDevice::openDevice()
{
    if (isDeviceOpen())
        return true;

    if (rc_port_name.isEmpty())
        return false;

    QSerialPort *port = new QSerialPort();
    port->setPortName(rc_port_name);
    port->setBaudRate(rc_baud_rate);

    if (!port->open(QIODevice::ReadWrite))
    {
        std::cout << "*** Failed to open serial port " << rc_port_name.toStdString()
                  << ": " << port->errorString().toStdString() << std::endl;
        delete port;
        return false;
    }

    connect(port, SIGNAL(readyRead()), this, SLOT(startRead()));

    rc_serial_port = port;

    return true;
}

void SerialRemoteControlDevice::closeDevice()
{
    if (rc_serial_port != 0)
    {
        rc_serial_port->close();
        rc_serial_port->deleteLater();
        rc_serial_port = 0;
    }
}

bool SerialRemoteControlDevice::isEnabledInSettings(QSettings *settings)
{
    QString settingPath = QString("%1/enabled").arg(SETTINGS_SECTION_NAME);
    return settings->value(settingPath, false).toBool();
}

void SerialRemoteControlDevice::readSettings(QSettings *settings)
{
    if (!settings)
        return;

    settings->beginGroup(SETTINGS_SECTION_NAME);

    if (settings->contains("port_name"))
        setPortName(settings->value("port_name").toString());

    if (settings->contains("baud_rate"))
        setBaudRate(settings->value("baud_rate").toInt());

    settings->endGroup();
}

void SerialRemoteControlDevice::saveSettings(QSettings *settings) const
{
    if (!settings)
        return;

    settings->beginGroup(SETTINGS_SECTION_NAME);

    if (isDeviceOpen())
        settings->setValue("enabled", true);
    else
        settings->remove("enabled");

    if (rc_port_name != DEFAULT_PORT_NAME)
        settings->setValue("port_name", rc_port_name);
    else
        settings->remove("port_name");

    if (rc_baud_rate != DEFAULT_BAUD_RATE)
        settings->setValue("baud_rate", rc_baud_rate);
    else
        settings->remove("baud_rate");

    settings->endGroup();
}

QString SerialRemoteControlDevice::getPortName() const
{
    return rc_port_name;
}

void SerialRemoteControlDevice::setPortName(QString port)
{
    if (port == rc_port_name)
        return;

    rc_port_name = port;
    if (isDeviceOpen())
    {
        closeDevice();
        openDevice();
    }
}

int SerialRemoteControlDevice::getBaudRate() const
{
    return rc_baud_rate;
}

void SerialRemoteControlDevice::setBaudRate(int baud_rate)
{
    if (baud_rate == rc_baud_rate)
        return;

    rc_baud_rate = baud_rate;
    if (isDeviceOpen())
    {
        closeDevice();
        openDevice();
    }
}

void SerialRemoteControlDevice::startRead()
{
    char    buffer[1024] = {0};
    int     bytes_read;

    bytes_read = rc_serial_port->readLine(buffer, 1024);
    if (bytes_read < 2)  // command + '\n'
        return;

    bool quit_requested = false;

    QString answer = remote_control->executeCommand(QString(buffer), quit_requested);

    if (quit_requested)
    {
        closeDevice();
        return;
    }

    if (!answer.isEmpty())
        rc_serial_port->write(answer.toLatin1());
}
