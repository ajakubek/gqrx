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
#ifndef TCP_REMOTE_CONTROL_SETTINGS_H
#define TCP_REMOTE_CONTROL_SETTINGS_H

#include <QDialog>
#include <QSettings>
#include <QStringList>

namespace Ui {
class TcpRemoteControlSettings;
}

/*! \brief Class to configure remote control settiongs. */
class TcpRemoteControlSettings : public QDialog
{
    Q_OBJECT
    
public:
    explicit TcpRemoteControlSettings(QWidget *parent = 0);
    ~TcpRemoteControlSettings();

    void setPort(int port);
    int  getPort(void) const;

    void setHosts(QStringList hosts);
    QStringList getHosts(void) const;
    
private slots:
    void on_hostAddButton_clicked(void);
    void on_hostDelButton_clicked(void);

private:
    Ui::TcpRemoteControlSettings *ui;
};

#endif // TCP_REMOTE_CONTROL_SETTINGS_H
