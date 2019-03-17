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
#include <QList>
#include <QListWidgetItem>

#include "tcp_remote_control_settings.h"
#include "ui_tcp_remote_control_settings.h"

TcpRemoteControlSettings::TcpRemoteControlSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TcpRemoteControlSettings)
{
    ui->setupUi(this);
}

TcpRemoteControlSettings::~TcpRemoteControlSettings()
{
    delete ui;
}

/*! \brief Set new network port.
 *  \param port The new network port.
 */
void TcpRemoteControlSettings::setPort(int port)
{
    ui->portSpinBox->setValue(port);
}

/*! \brief Get current value from the port spin box.
 *  \return The current port value.
 */
int TcpRemoteControlSettings::getPort(void) const
{
    return ui->portSpinBox->value();
}

/*! \brief Add items to the list of allowed hosts.
 *  \param hosts A list with the IP addresses of the allowed hosts.
 *
 * Note that setting the list wil lclear the current contents of the
 * list widget.
 */
void TcpRemoteControlSettings::setHosts(QStringList hosts)
{
    ui->hostListWidget->clear();
    ui->hostListWidget->addItems(hosts);
}

/*! \brief Get list of allowed hosts. */
QStringList TcpRemoteControlSettings::getHosts(void) const
{
    QStringList list;

    for (int i = 0; i < ui->hostListWidget->count(); i++)
    {
        list << ui->hostListWidget->item(i)->text();
    }

    return list;
}


/*! \brief Add a new entry to the list of allowed hosts.
 *
 * The function inserts a  new entry at the end of the list and enables
 * editing.
 */
void TcpRemoteControlSettings::on_hostAddButton_clicked(void)
{
    ui->hostListWidget->addItem(tr("Enter IP"));

    QListWidgetItem *item = ui->hostListWidget->item(ui->hostListWidget->count()-1);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ui->hostListWidget->setCurrentItem(item, QItemSelectionModel::ClearAndSelect);
    ui->hostListWidget->editItem(item);
}

/*! \brief Delete the selected entries from the list of allowed hosts. */
void TcpRemoteControlSettings::on_hostDelButton_clicked(void)
{
    // wondering WTF?
    // see http://stackoverflow.com/questions/7008423/how-do-i-remove-all-the-selected-items-in-a-qlistwidget
    qDeleteAll(ui->hostListWidget->selectedItems());
}


