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
#ifndef SERIAL_REMOTE_CONTROL_DEVICE_H
#define SERIAL_REMOTE_CONTROL_DEVICE_H

#include <QObject>
#include <QStringList>

class QSettings;
class QSerialPort;

class RemoteControl;

class SerialRemoteControlDevice : public QObject
{
    Q_OBJECT
public:
    SerialRemoteControlDevice(RemoteControl *remote_control, QObject *parent = 0);
    ~SerialRemoteControlDevice();

    bool isDeviceOpen() const;

    bool openDevice();
    void closeDevice();

    static bool isEnabledInSettings(QSettings *settings);
    void readSettings(QSettings *settings);
    void saveSettings(QSettings *settings) const;

    QString getPortName() const;
    void setPortName(QString portName);

    int getBaudRate() const;
    void setBaudRate(int baudRate);

private slots:
    void startRead();

private:
    void readSettingsFromSection(QSettings* settings, QString section_name);

    RemoteControl*  remote_control;

    QSerialPort*    rc_serial_port; /*!< Serial port for remote control device. */

    QString         rc_port_name;   /*!< Hosts where we accept connection from. */
    int             rc_baud_rate;   /*!< The port we are listening on. */
};

#endif // SERIAL_REMOTE_CONTROL_DEVICE_H
