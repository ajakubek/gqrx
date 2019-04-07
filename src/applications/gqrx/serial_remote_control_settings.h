#ifndef SERIAL_REMOTE_CONTROL_SETTINGS_H
#define SERIAL_REMOTE_CONTROL_SETTINGS_H

#include <QDialog>

namespace Ui {
class SerialRemoteControlSettings;
}

class SerialRemoteControlSettings : public QDialog
{
    Q_OBJECT

public:
    explicit SerialRemoteControlSettings(QWidget *parent = nullptr);
    ~SerialRemoteControlSettings();

    void setPortName(QString portName);
    QString getPortName() const;

    void setBaudRate(int baudRate);
    int getBaudRate() const;

private:
    void detectPortNames();
    void detectBaudRates();

    Ui::SerialRemoteControlSettings *ui;
};

#endif // SERIAL_REMOTE_CONTROL_SETTINGS_H
