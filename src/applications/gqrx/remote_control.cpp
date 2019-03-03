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
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include "remote_control.h"

RemoteControl::RemoteControl(QObject *parent) :
    QObject(parent)
{
    rc_freq = 0;
    rc_filter_offset = 0;
    bw_half = 740e3;
    rc_lnb_lo_mhz = 0.0;
    rc_mode = 0;
    rc_passband_lo = 0;
    rc_passband_hi = 0;
    signal_level = -200.0;
    squelch_level = -150.0;
    audio_recorder_status = false;
    receiver_running = false;
    hamlib_compatible = false;
}

RemoteControl::~RemoteControl()
{
}

/*! \brief Execute command and return response */
QString RemoteControl::executeCommand(QString command, bool &quit_requested)
{
    QString answer = "";

    quit_requested = false;

    if (command.length() < 2)  // command + '\n'
        return answer;

    QStringList cmdlist = command.trimmed().split(" ", QString::SkipEmptyParts);

    if (cmdlist.size() == 0)
        return answer;

    QString cmd = cmdlist[0];
    if (cmd == "f")
        answer = cmd_get_freq();
    else if (cmd == "F")
        answer = cmd_set_freq(cmdlist);
    else if (cmd == "m")
        answer = cmd_get_mode();
    else if (cmd == "M")
        answer = cmd_set_mode(cmdlist);
    else if (cmd == "l")
        answer = cmd_get_level(cmdlist);
    else if (cmd == "L")
        answer = cmd_set_level(cmdlist);
    else if (cmd == "u")
        answer = cmd_get_func(cmdlist);
    else if (cmd == "U")
        answer = cmd_set_func(cmdlist);
    else if (cmd == "v")
        answer = cmd_get_vfo();
    else if (cmd == "V")
        answer = cmd_set_vfo(cmdlist);
    else if (cmd == "s")
        answer = cmd_get_split_vfo();
    else if (cmd == "S")
        answer = cmd_set_split_vfo();
    else if (cmd == "_")
        answer = cmd_get_info();
    else if (cmd == "AOS")
        answer = cmd_AOS();
    else if (cmd == "LOS")
        answer = cmd_LOS();
    else if (cmd == "LNB_LO")
        answer = cmd_lnb_lo(cmdlist);
    else if (cmd == "\\dump_state")
        answer = cmd_dump_state();
    else if (cmd == "q" || cmd == "Q")
    {
        // FIXME: for now we assume 'close' command
        quit_requested = true;
    }
    else
    {
        // print unknown command and respond with an error
        qWarning() << "Unknown remote command:" << cmdlist;
        answer = QString("RPRT 1\n");
    }

    return answer;
}

/*! \brief Slot called when the receiver is tuned to a new frequency.
 *  \param freq The new frequency in Hz.
 *
 * Note that this is the frequency gqrx is receiveing on, i.e. the
 * hardware frequency + the filter offset.
 */
void RemoteControl::setNewFrequency(qint64 freq)
{
    rc_freq = freq;
}

/*! \brief Slot called when the filter offset is changed. */
void RemoteControl::setFilterOffset(qint64 freq)
{
    rc_filter_offset = freq;
}

/*! \brief Slot called when the LNB LO frequency has changed
 *  \param freq_mhz new LNB LO frequency in MHz
 */
void RemoteControl::setLnbLo(double freq_mhz)
{
    rc_lnb_lo_mhz = freq_mhz;
}

void RemoteControl::setBandwidth(qint64 bw)
{
    // we want to leave some margin
    bw_half = (qint64)(0.9f * (bw / 2.f));
}

/*! \brief Set signal level in dBFS. */
void RemoteControl::setSignalLevel(float level)
{
    signal_level = level;
}

/*! \brief Set demodulator (from mainwindow). */
void RemoteControl::setMode(int mode)
{
    rc_mode = mode;

    if (rc_mode == 0)
        audio_recorder_status = false;
}

/*! \brief Set passband (from mainwindow). */
void RemoteControl::setPassband(int passband_lo, int passband_hi)
{
    rc_passband_lo = passband_lo;
    rc_passband_hi = passband_hi;
}

/*! \brief New remote frequency received. */
void RemoteControl::setNewRemoteFreq(qint64 freq)
{
    qint64 delta = freq - rc_freq;
    qint64 bwh_eff = 0.8f * (float)bw_half;

    rc_filter_offset += delta;
    if ((rc_filter_offset > 0 && rc_filter_offset + rc_passband_hi < bwh_eff) ||
        (rc_filter_offset < 0 && rc_filter_offset + rc_passband_lo > -bwh_eff))
    {
        // move filter offset
        emit newFilterOffset(rc_filter_offset);
    }
    else
    {
        // moving filter offset would push it too close to or beyond the edge
        // move it close to the center and adjust hardware freq
        if (rc_filter_offset < 0)
            rc_filter_offset = -0.2f * bwh_eff;
        else
            rc_filter_offset = 0.2f * bwh_eff;
        emit newFilterOffset(rc_filter_offset);
        emit newFrequency(freq);
    }

    rc_freq = freq;
}

/*! \brief Set squelch level (from mainwindow). */
void RemoteControl::setSquelchLevel(double level)
{
    squelch_level = level;
}

/*! \brief Start audio recorder (from mainwindow). */
void RemoteControl::startAudioRecorder(QString unused)
{
    if (rc_mode > 0)
        audio_recorder_status = true;
}

/*! \brief Stop audio recorder (from mainwindow). */
void RemoteControl::stopAudioRecorder()
{
    audio_recorder_status = false;
}

/*! \brief Set receiver status (from mainwindow). */
void RemoteControl::setReceiverStatus(bool enabled)
{
    receiver_running = enabled;
}


/*! \brief Convert mode string to enum (DockRxOpt::rxopt_mode_idx)
 *  \param mode The Hamlib rigctld compatible mode string
 *  \return An integer corresponding to the mode.
 *
 * Following mode strings are recognized: OFF, RAW, AM, FM, WFM,
 * WFM_ST, WFM_ST_OIRT, LSB, USB, CW, CWL, CWU.
 */
int RemoteControl::modeStrToInt(QString mode_str)
{
    int mode_int = -1;

    if (mode_str.compare("OFF", Qt::CaseInsensitive) == 0)
    {
        mode_int = 0;
    }
    else if (mode_str.compare("RAW", Qt::CaseInsensitive) == 0)
    {
        mode_int = 1;
    }
    else if (mode_str.compare("AM", Qt::CaseInsensitive) == 0)
    {
        mode_int = 2;
    }
    else if (mode_str.compare("FM", Qt::CaseInsensitive) == 0)
    {
        mode_int = 3;
    }
    else if (mode_str.compare("WFM", Qt::CaseInsensitive) == 0)
    {
        mode_int = 4;
    }
    else if (mode_str.compare("WFM_ST", Qt::CaseInsensitive) == 0)
    {
        mode_int = 5;
    }
    else if (mode_str.compare("LSB", Qt::CaseInsensitive) == 0)
    {
        mode_int = 6;
    }
    else if (mode_str.compare("USB", Qt::CaseInsensitive) == 0)
    {
        mode_int = 7;
    }
    else if (mode_str.compare("CW", Qt::CaseInsensitive) == 0)
    {
        mode_int = 9;
        hamlib_compatible = true;
    }
    else if (mode_str.compare("CWL", Qt::CaseInsensitive) == 0)
    {
        mode_int = 8;
        hamlib_compatible = false;
    }
    else if (mode_str.compare("CWR", Qt::CaseInsensitive) == 0)
    {
        mode_int = 8;
        hamlib_compatible = true;
    }
    else if (mode_str.compare("CWU", Qt::CaseInsensitive) == 0)
    {
        mode_int = 9;
        hamlib_compatible = false;
    }
    else if (mode_str.compare("WFM_ST_OIRT", Qt::CaseInsensitive) == 0)
    {
        mode_int = 10;
    }

    return mode_int;
}

/*! \brief Convert mode enum to string.
 *  \param mode The mode ID c.f. DockRxOpt::rxopt_mode_idx
 *  \returns The mode string.
 */
QString RemoteControl::intToModeStr(int mode)
{
    QString mode_str;

    switch (mode)
    {
    case 0:
        mode_str = "OFF";
        break;

    case 1:
        mode_str = "RAW";
        break;

    case 2:
        mode_str = "AM";
        break;

    case 3:
        mode_str = "FM";
        break;

    case 4:
        mode_str = "WFM";
        break;

    case 5:
        mode_str = "WFM_ST";
        break;

    case 6:
        mode_str = "LSB";
        break;

    case 7:
        mode_str = "USB";
        break;

    case 8:
        mode_str = (hamlib_compatible) ? "CWR" : "CWL";
        break;

    case 9:
        mode_str = (hamlib_compatible) ? "CW" : "CWU";
        break;

    case 10:
        mode_str = "WFM_ST_OIRT";
        break;

    default:
        mode_str = "ERR";
        break;
    }

    return mode_str;
}

/* Get frequency */
QString RemoteControl::cmd_get_freq() const
{
    return QString("%1\n").arg(rc_freq);
}

/* Set new frequency */
QString RemoteControl::cmd_set_freq(QStringList cmdlist)
{
    bool ok;
    double freq = cmdlist.value(1, "ERR").toDouble(&ok);

    if (ok)
    {
        setNewRemoteFreq((qint64)freq);
        return QString("RPRT 0\n");
    }

    return QString("RPRT 1\n");
}

/* Get mode and passband */
QString RemoteControl::cmd_get_mode()
{
    return QString("%1\n%2\n")
                   .arg(intToModeStr(rc_mode))
                   .arg(rc_passband_hi - rc_passband_lo);
}

/* Set mode and passband */
QString RemoteControl::cmd_set_mode(QStringList cmdlist)
{
    QString answer;
    QString cmd_arg = cmdlist.value(1, "");

    if (cmd_arg == "?")
        answer = QString("OFF RAW AM FM WFM WFM_ST WFM_ST_OIRT LSB USB CW CWU CWR CWL\n");
    else
    {
        int mode = modeStrToInt(cmd_arg);
        if (mode == -1)
        {
            // invalid mode string
            answer = QString("RPRT 1\n");
        }
        else
        {
            rc_mode = mode;
            emit newMode(rc_mode);

            int passband = cmdlist.value(2, "0").toInt();
            if ( passband != 0 )
                emit newPassband(passband);

            if (rc_mode == 0)
                audio_recorder_status = false;

            answer = QString("RPRT 0\n");
        }
    }
    return answer;
}

/* Get level */
QString RemoteControl::cmd_get_level(QStringList cmdlist)
{
    QString answer;
    QString lvl = cmdlist.value(1, "");

    if (lvl == "?")
       answer = QString("SQL STRENGTH\n");
    else if (lvl.compare("STRENGTH", Qt::CaseInsensitive) == 0 || lvl.isEmpty())
       answer = QString("%1\n").arg(signal_level, 0, 'f', 1);
    else if (lvl.compare("SQL", Qt::CaseInsensitive) == 0)
       answer = QString("%1\n").arg(squelch_level, 0, 'f', 1);
    else
       answer = QString("RPRT 1\n");

    return answer;
}

/* Set level */
QString RemoteControl::cmd_set_level(QStringList cmdlist)
{
    QString answer;
    QString lvl = cmdlist.value(1, "");

    if (lvl == "?")
        answer = QString("SQL\n");
    else if (lvl.compare("SQL", Qt::CaseInsensitive) == 0)
    {
        bool ok;
        double squelch = cmdlist.value(2, "ERR").toDouble(&ok);
        if (ok)
        {
            answer = QString("RPRT 0\n");
            squelch_level = std::max<double>(-150, std::min<double>(0, squelch));
            emit newSquelchLevel(squelch_level);
        }
        else
        {
            answer = QString("RPRT 1\n");
        }
    }
    else
    {
        answer = QString("RPRT 1\n");
    }

    return answer;
}

/* Get function */
QString RemoteControl::cmd_get_func(QStringList cmdlist)
{
    QString answer;
    QString func = cmdlist.value(1, "");

    if (func == "?")
        answer = QString("RECORD\n");
    else if (func.compare("RECORD", Qt::CaseInsensitive) == 0)
        answer = QString("%1\n").arg(audio_recorder_status);
    else
        answer = QString("RPRT 1\n");

    return answer;
}

/* Set function */
QString RemoteControl::cmd_set_func(QStringList cmdlist)
{
    bool ok;
    QString answer;
    QString func = cmdlist.value(1, "");
    int     status = cmdlist.value(2, "ERR").toInt(&ok);

    if (func == "?")
    {
        answer = QString("RECORD\n");
    }
    else if ((func.compare("RECORD", Qt::CaseInsensitive) == 0) && ok)
    {
        if (rc_mode == 0 || !receiver_running)
        {
            answer = QString("RPRT 1\n");
        }
        else
        {
            answer = QString("RPRT 0\n");
            audio_recorder_status = status;
            if (status)
                emit startAudioRecorderEvent();
            else
                emit stopAudioRecorderEvent();
        }
    }
    else
    {
        answer = QString("RPRT 1\n");
    }

    return answer;
}

/* Get current 'VFO' (fake, only for hamlib) */
QString RemoteControl::cmd_get_vfo() const
{
    return QString("VFOA\n");
};

/* Set 'VFO' (fake, only for hamlib) */
QString RemoteControl::cmd_set_vfo(QStringList cmdlist)
{
    QString cmd_arg = cmdlist.value(1, "");
    QString answer;

    if (cmd_arg == "?")
        answer = QString("VFOA\n");
    else if (cmd_arg == "VFOA")
        answer = QString("RPRT 0\n");
    else
        answer = QString("RPRT 1\n");

    return answer;
};

/* Get 'Split' mode (fake, only for hamlib) */
QString RemoteControl::cmd_get_split_vfo() const
{
    return QString("0\nVFOA\n");
};

/* Set 'Split' mode (fake, only for hamlib) */
QString RemoteControl::cmd_set_split_vfo()
{
    return QString("RPRT 1\n");
}

/* Get info */
QString RemoteControl::cmd_get_info() const
{
    return QString("Gqrx %1\n").arg(VERSION);
};

/* Gpredict / Gqrx specific command: AOS - satellite AOS event */
QString RemoteControl::cmd_AOS()
{
    if (rc_mode > 0 && receiver_running)
    {
        emit startAudioRecorderEvent();
        audio_recorder_status = true;
    }
    return QString("RPRT 0\n");
}

/* Gpredict / Gqrx specific command: LOS - satellite LOS event */
QString RemoteControl::cmd_LOS()
{
    emit stopAudioRecorderEvent();
    audio_recorder_status = false;
    return QString("RPRT 0\n");
}

/* Set the LNB LO value */
QString RemoteControl::cmd_lnb_lo(QStringList cmdlist)
{
    if(cmdlist.size() == 2)
    {
        bool ok;
        qint64 freq = cmdlist[1].toLongLong(&ok);

        if (ok)
        {
            rc_lnb_lo_mhz = freq / 1e6;
            emit newLnbLo(rc_lnb_lo_mhz);
            return QString("RPRT 0\n");
        }

        return QString("RPRT 1\n");
    }
    else
    {
        return QString("%1\n").arg((qint64)(rc_lnb_lo_mhz * 1e6));
    }
}

/*
 * '\dump_state' used by hamlib clients, e.g. xdx, fldigi, rigctl and etc
 * More info:
 *  https://github.com/N0NB/hamlib/blob/master/include/hamlib/rig.h (bit fields)
 *  https://github.com/N0NB/hamlib/blob/master/dummy/netrigctl.c
 */
QString RemoteControl::cmd_dump_state() const
{
    return QString(
        /* rigctl protocol version */
        "0\n"
        /* rigctl model */
        "2\n"
        /* ITU region */
        "1\n"
        /* RX/TX frequency ranges
         * start, end, modes, low_power, high_power, vfo, ant
         *  start/end - Start/End frequency [Hz]
         *  modes - Bit field of RIG_MODE's (AM|CW|CWR|USB|LSB|FM|WFM)
         *  low_power/high_power - Lower/Higher RF power in mW,
         *                         -1 for no power (ie. rx list)
         *  vfo - VFO list equipped with this range (RIG_VFO_A)
         *  ant - Antenna list equipped with this range, 0 means all
         *  FIXME: limits can be gets from receiver::get_rf_range()
         */
        "0.000000 10000000000.000000 0xef -1 -1 0x1 0x0\n"
        /* End of RX frequency ranges. */
        "0 0 0 0 0 0 0\n"
        /* End of TX frequency ranges. The Gqrx is reciver only. */
        "0 0 0 0 0 0 0\n"
        /* Tuning steps: modes, tuning_step */
        "0xef 1\n"
        "0xef 0\n"
        /* End of tuning steps */
        "0 0\n"
        /* Filter sizes: modes, width
         * FIXME: filter can be gets from filter_preset_table
         */
        "0x82 500\n"    /* CW | CWR normal */
        "0x82 200\n"    /* CW | CWR narrow */
        "0x82 2000\n"   /* CW | CWR wide */
        "0x21 10000\n"  /* AM | FM normal */
        "0x21 5000\n"   /* AM | FM narrow */
        "0x21 20000\n"  /* AM | FM wide */
        "0x0c 2700\n"   /* SSB normal */
        "0x0c 1400\n"   /* SSB narrow */
        "0x0c 3900\n"   /* SSB wide */
        "0x40 160000\n" /* WFM normal */
        "0x40 120000\n" /* WFM narrow */
        "0x40 200000\n" /* WFM wide */
        /* End of filter sizes  */
        "0 0\n"
        /* max_rit  */
        "0\n"
        /* max_xit */
        "0\n"
        /* max_ifshift */
        "0\n"
        /* Announces (bit field list) */
        "0\n" /* RIG_ANN_NONE */
        /* Preamp list in dB, 0 terminated */
        "0\n"
        /* Attenuator list in dB, 0 terminated */
        "0\n"
        /* Bit field list of get functions */
        "0\n" /* RIG_FUNC_NONE */
        /* Bit field list of set functions */
        "0\n" /* RIG_FUNC_NONE */
        /* Bit field list of get level */
        "0x40000020\n" /* RIG_LEVEL_SQL | RIG_LEVEL_STRENGTH */
        /* Bit field list of set level */
        "0x20\n"       /* RIG_LEVEL_SQL */
        /* Bit field list of get parm */
        "0\n" /* RIG_PARM_NONE */
        /* Bit field list of set parm */
        "0\n" /* RIG_PARM_NONE */);
}
