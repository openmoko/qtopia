/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qmediapipe.h>

#include "qtopiaaudiooutputdevice.h"


QtopiaAudioOutputDevice::QtopiaAudioOutputDevice(QMap<QString, QVariant> const& options)
{
    if (!options.isEmpty())
    {
        QMap<QString, QVariant>::const_iterator   it;

        for (it = options.begin(); it != options.end(); ++it)
        {
            setValue(it.key(), it.value());
        }
    }
    else
    {
        m_audioOutput.setFrequency(44100);
        m_audioOutput.setChannels(2);
        m_audioOutput.setBitsPerSample(16);
    }
}

QtopiaAudioOutputDevice::~QtopiaAudioOutputDevice()
{
}

void QtopiaAudioOutputDevice::setInputPipe(QMediaPipe* inputPipe)
{
    m_inputPipe = inputPipe;

    connect(m_inputPipe, SIGNAL(readyRead()),
            this, SLOT(dataReady()));
}

void QtopiaAudioOutputDevice::setOutputPipe(QMediaPipe*)
{
    qWarning("QtopiaAudioInputDevice: Device is a sink; setting the output is not valid");
}

void QtopiaAudioOutputDevice::setValue
(
 QString const& name,
 QVariant const& value
)
{
    if (name == "frequency")
        m_audioOutput.setFrequency(value.toInt());
    else
    if (name == "channels")
        m_audioOutput.setChannels(value.toInt());
    else
    if (name == "bitsPerSample")
        m_audioOutput.setBitsPerSample(value.toInt());
}

QVariant QtopiaAudioOutputDevice::value(QString const& name)
{
    QVariant    rc;

    if (name == "frequency")
        rc = m_audioOutput.frequency();
    else
    if (name == "channels")
        rc = m_audioOutput.channels();
    else
    if (name == "bitsPerSample")
        rc = m_audioOutput.bitsPerSample();

    return rc;
}


bool QtopiaAudioOutputDevice::open(QIODevice::OpenMode mode)
{
    return m_audioOutput.open(mode);
}

void QtopiaAudioOutputDevice::close()
{
    m_audioOutput.close();
}

bool QtopiaAudioOutputDevice::isSequential() const
{
    return m_audioOutput.isSequential();
}

qint64 QtopiaAudioOutputDevice::readData( char *data, qint64 maxlen )
{
    return m_audioOutput.read(data, maxlen);
}

qint64 QtopiaAudioOutputDevice::writeData(const char *data, qint64 len)
{
    return m_audioOutput.write(data, len);
}

void QtopiaAudioOutputDevice::dataReady()
{
    const int buffer_size = 1024 * 8;

    char    buf[buffer_size];

    qint64  rc = m_inputPipe->read(buf, buffer_size);

    if (rc > 0)
    {
        m_audioOutput.write(buf, rc);
    }
}

