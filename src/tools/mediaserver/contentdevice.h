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

#ifndef __QTOPIA_MEDIASERVER_CONTENTDEVICE_H
#define __QTOPIA_MEDIASERVER_CONTENTDEVICE_H

#include <qmediadevice.h>


class ContentDevicePrivate;

class ContentDevice : public QMediaDevice
{
public:
    ContentDevice(QString const& filePath);
    ~ContentDevice();

    void setInputPipe(QMediaPipe* inputtPipe);
    void setOutputPipe(QMediaPipe* outputPipe);

    void setValue(QString const& name, QVariant const& value);
    QVariant value(QString const& name);

    bool open(QIODevice::OpenMode mode);
    void close();
    bool isSequential() const;

protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );

private:
    ContentDevicePrivate*   d;
};


#endif  // __QTOPIA_MEDIASERVER_CONTENTDEVICE_H
