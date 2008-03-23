/***************************************************************************
**
** Copyright (C) 2008 Openmoko Inc.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2 or at your option any later version.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************/

#include <qtopiacomm/qserialiodevicemultiplexer.h>
#include <qtopiacomm/qatchat.h>
#include <qtopiacomm/qatresult.h>

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>


class SerialDeviceTester : public QObject {
    Q_OBJECT
public:
    SerialDeviceTester(QSerialIODevice* iodevice);
    void startTesting();

public Q_SLOTS:
    void sendRequest();
    void result(bool,QAtResult);

private:
    QSerialIODevice* m_device;
};


SerialDeviceTester::SerialDeviceTester(QSerialIODevice* iodevice)
    : m_device(iodevice)
{
    Q_ASSERT(m_device);
    Q_ASSERT(m_device->atchat());
}

void SerialDeviceTester::startTesting()
{
    sendRequest();
}

void SerialDeviceTester::sendRequest()
{
    m_device->atchat()->chat("AT+CGSN", this, SLOT(result(bool,QAtResult)));
}

void SerialDeviceTester::result(bool,QAtResult result)
{
    qDebug() << "Result:" << result.verboseResult();
    QTimer::singleShot(5*1000, this, SLOT(sendRequest()));
}


int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    QSerialIODeviceMultiplexer* mux = QSerialIODeviceMultiplexer::create();
    if (!mux) {
        qDebug() << "Could not create a muxer";
        return EXIT_FAILURE;
    }

    SerialDeviceTester tester(mux->channel(QLatin1String("primary")));
    tester.startTesting();

    return app.exec();
}

#include "attest.moc"
