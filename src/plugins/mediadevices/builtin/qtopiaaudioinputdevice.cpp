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

#include "qtopiaaudioinputdevice.h"

QtopiaAudioInputDevice::QtopiaAudioInputDevice(QMap<QString, QVariant> const& options)
{
    QMap<QString, QVariant>::const_iterator   it;

    for (it = options.begin(); it != options.end(); ++it)
    {
        setValue(it.key(), it.value());
    }
}

QtopiaAudioInputDevice::~QtopiaAudioInputDevice()
{
}

void QtopiaAudioInputDevice::setInputPipe(QMediaPipe*)
{
    qWarning("QtopiaAudioInputDevice: Device is a source; setting the input is not valid");
}

void QtopiaAudioInputDevice::setOutputPipe(QMediaPipe* outputPipe)
{
    m_outputPipe = outputPipe;
}

void QtopiaAudioInputDevice::setValue(QString const& name, QVariant const& value)
{
    if (name == "frequency")
        m_audioInput.setFrequency(value.toInt());
    else
    if (name == "channels")
        m_audioInput.setChannels(value.toInt());
    else
    if (name == "samplesPerBlock")
        m_audioInput.setSamplesPerBlock(value.toInt());
}

QVariant QtopiaAudioInputDevice::value(QString const& name)
{
    QVariant    rc;

    if (name == "frequency")
        rc = m_audioInput.frequency();
    else
    if (name == "channels")
        rc = m_audioInput.channels();
    else
    if (name == "samplesPerBlock")
        rc = m_audioInput.samplesPerBlock();

    return rc;
}


bool QtopiaAudioInputDevice::open(QIODevice::OpenMode mode)
{
    return m_audioInput.open(mode);
}

void QtopiaAudioInputDevice::close()
{
    m_audioInput.close();
}

bool QtopiaAudioInputDevice::isSequential() const
{
    return m_audioInput.isSequential();
}

qint64 QtopiaAudioInputDevice::readData( char *data, qint64 maxlen )
{
    return m_audioInput.read(data, maxlen);
}

qint64 QtopiaAudioInputDevice::writeData(const char *data, qint64 len)
{
    return m_audioInput.write(data, len);
}



