/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef __QTOPiA_MEDIADEVICES_QTOPIAAUDIOOUTPUTDEVICE_H
#define __QTOPiA_MEDIADEVICES_QTOPIAAUDIOOUTPUTDEVICE_H

#include <qstring.h>
#include <qvariant.h>
#include <qmap.h>

#include <qtopiamedia/qmediadevice.h>
#include <qtopiaaudio/qaudiooutput.h>

class QMediaPipe;

class QtopiaAudioOutputDevice : public QMediaDevice
{
    Q_OBJECT

public:
    QtopiaAudioOutputDevice(QMap<QString, QVariant> const& options);
    ~QtopiaAudioOutputDevice();

    void setInputPipe(QMediaPipe* inputPipe);
    void setOutputPipe(QMediaPipe* outputPipe);

    void setValue(QString const& name, QVariant const& value);
    QVariant value(QString const& name);

    bool open(QIODevice::OpenMode mode);
    void close();
    bool isSequential() const;

protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );

private slots:
    void dataReady();

private:
    QMediaPipe*     m_inputPipe;
    QAudioOutput    m_audioOutput;
};

#endif  // __QTOPiA_MEDIADEVICES_QTOPIAAUDIOOUTPUTDEVICE_H

