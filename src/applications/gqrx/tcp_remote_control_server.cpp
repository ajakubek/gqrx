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
#include "tcp_remote_control_server.h"

#include <iostream>

#include <QSettings>
#include <QTcpSocket>

#include "remote_control.h"

#define DEFAULT_RC_PORT            7356
#define DEFAULT_RC_ALLOWED_HOSTS   "::ffff:127.0.0.1"

TcpRemoteControlServer::TcpRemoteControlServer(RemoteControl *remote_control,
                                               QObject *parent)
    : QObject(parent), remote_control(remote_control)
{
    rc_port = DEFAULT_RC_PORT;
    rc_allowed_hosts.append(DEFAULT_RC_ALLOWED_HOSTS);

    rc_socket = 0;

#if QT_VERSION < 0x050900
    // Disable proxy setting detected by Qt
    // Workaround for https://bugreports.qt.io/browse/QTBUG-58374
    // Fix: https://codereview.qt-project.org/#/c/186124/
    rc_server.setProxy(QNetworkProxy::NoProxy);
#endif

    connect(&rc_server, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
}

TcpRemoteControlServer::~TcpRemoteControlServer()
{
    stopServer();
}

void TcpRemoteControlServer::startServer()
{
    if (!rc_server.isListening())
        rc_server.listen(QHostAddress::Any, rc_port);
}

void TcpRemoteControlServer::stopServer()
{
    if (rc_socket != 0)
    {
        rc_socket->close();
        rc_socket->deleteLater();
        rc_socket = 0;
    }

    if (rc_server.isListening())
        rc_server.close();
}

bool TcpRemoteControlServer::isEnabledInSettings(QSettings *settings)
{
    if (settings->contains("remote_control/enabled"))
        return settings->value("remote_control/enabled", false).toBool();

    return settings->value("tcp_remote_control_server/enabled", false).toBool();
}

void TcpRemoteControlServer::readSettings(QSettings *settings)
{
    readSettingsFromSection(settings, "remote_control");  // legacy settings
    readSettingsFromSection(settings, "tcp_remote_control_server");
}

void TcpRemoteControlServer::saveSettings(QSettings *settings) const
{
    if (!settings)
        return;

    settings->beginGroup("tcp_remote_control_server");

    if (rc_server.isListening())
        settings->setValue("enabled", true);
    else
        settings->remove("enabled");

    if (rc_port != DEFAULT_RC_PORT)
        settings->setValue("port", rc_port);
    else
        settings->remove("port");

    if (rc_allowed_hosts.count() > 0)
        settings->setValue("allowed_hosts", rc_allowed_hosts);
    else
        settings->remove("allowed_hosts");

    settings->endGroup();
}

/*! \brief Set new network port.
 *  \param port The new network port.
 *
 * If the server is running it will be restarted.
 *
 */
void TcpRemoteControlServer::setPort(int port)
{
    if (port == rc_port)
        return;

    rc_port = port;
    if (rc_server.isListening())
    {
        rc_server.close();
        rc_server.listen(QHostAddress::Any, rc_port);
    }
}

int  TcpRemoteControlServer::getPort() const
{
    return rc_port;
}

void TcpRemoteControlServer::setHosts(QStringList hosts)
{
    rc_allowed_hosts = hosts;
}

QStringList TcpRemoteControlServer::getHosts() const
{
    return rc_allowed_hosts;
}

/*! \brief Accept a new client connection.
 *
 * This slot is called when a client opens a new connection.
 */
void TcpRemoteControlServer::acceptConnection()
{
    if (rc_socket)
    {
        rc_socket->close();
        rc_socket->deleteLater();
    }
    rc_socket = rc_server.nextPendingConnection();

    // check if host is allowed
    QString address = rc_socket->peerAddress().toString();
    if (rc_allowed_hosts.indexOf(address) == -1)
    {
        std::cout << "*** Remote connection attempt from " << address.toStdString()
                  << " (not in allowed list)" << std::endl;
        rc_socket->close();
        rc_socket->deleteLater();
        rc_socket = 0;
    }
    else
    {
        connect(rc_socket, SIGNAL(readyRead()), this, SLOT(startRead()));
    }
}

/*! \brief Start reading from the socket.
 *
 * This slot is called when the client TCP socket emits a readyRead() signal,
 * i.e. when there is data to read.
 */
void TcpRemoteControlServer::startRead()
{
    char    buffer[1024] = {0};
    int     bytes_read;

    bytes_read = rc_socket->readLine(buffer, 1024);
    if (bytes_read < 2)  // command + '\n'
        return;

    bool quit_requested = false;

    QString answer = remote_control->executeCommand(QString(buffer), quit_requested);

    if (quit_requested)
    {
        rc_socket->close();
        rc_socket->deleteLater();
        rc_socket = 0;
        return;
    }

    if (!answer.isEmpty())
        rc_socket->write(answer.toLatin1());
}

void TcpRemoteControlServer::readSettingsFromSection(QSettings* settings,
                                                     QString section_name)
{
    if (!settings)
        return;

    settings->beginGroup(section_name);

    if (settings->contains("port"))
        setPort(settings->value("port").toInt());

    if (settings->contains("allowed_hosts"))
        setHosts(settings->value("allowed_hosts").toStringList());

    settings->endGroup();
}
