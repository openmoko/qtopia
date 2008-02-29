/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef QPASSTHROUGHSERIALIODEVICE_P_H
#define QPASSTHROUGHSERIALIODEVICE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qserialiodevice.h>

class QPassThroughSerialIODevice : public QSerialIODevice
{
    Q_OBJECT
public:
    explicit QPassThroughSerialIODevice( QSerialIODevice *device, QObject *parent = 0 );
    ~QPassThroughSerialIODevice();

    bool isEnabled() const { return enabled; }
    void setEnabled( bool flag ) { enabled = flag; }

    bool open( OpenMode mode );
    void close();
    bool waitForReadyRead(int msecs);
    qint64 bytesAvailable() const;

    int rate() const;
    bool dtr() const;
    void setDtr( bool value );
    bool dsr() const;
    bool carrier() const;
    bool setCarrier( bool value );
    bool rts() const;
    void setRts( bool value );
    bool cts() const;

    void discard();

    bool isValid() const;

signals:
    void opened();
    void closed();

protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );

private slots:
    void deviceReadyRead();

private:
    QSerialIODevice *device;
    bool enabled;
};

#endif // QPASSTHROUGHSERIALIODEVICE_P_H
