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
#ifndef TCP_REMOTE_CONTROL_SERVER_H
#define TCP_REMOTE_CONTROL_SERVER_H

#include <QObject>
#include <QStringList>
#include <QTcpServer>

class QSettings;
class QTcpSocket;

class RemoteControl;

class TcpRemoteControlServer : public QObject
{
    Q_OBJECT
public:
    TcpRemoteControlServer(RemoteControl *remote_control, QObject *parent = 0);
    ~TcpRemoteControlServer();

    void startServer();
    void stopServer();

    static bool isEnabledInSettings(QSettings *settings);
    void readSettings(QSettings *settings);
    void saveSettings(QSettings *settings) const;

    void setPort(int port);
    int  getPort() const;

    void setHosts(QStringList hosts);
    QStringList getHosts() const;

private slots:
    void acceptConnection();
    void startRead();

private:
    void readSettingsFromSection(QSettings* settings, QString section_name);

    RemoteControl*  remote_control;

    QTcpServer      rc_server;         /*!< The active server object. */
    QTcpSocket*     rc_socket;         /*!< The active socket object. */

    QStringList     rc_allowed_hosts;  /*!< Hosts where we accept connection from. */
    int             rc_port;           /*!< The port we are listening on. */
};

#endif // TCP_REMOTE_CONTROL_SERVER_H
