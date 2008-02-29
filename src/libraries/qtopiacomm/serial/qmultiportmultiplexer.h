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

#ifndef QMULTIPORTMULTIPLEXER_H
#define QMULTIPORTMULTIPLEXER_H

#include <qserialiodevicemultiplexer.h>

class QMultiPortMultiplexerPrivate;

class QTOPIACOMM_EXPORT QMultiPortMultiplexer : public QSerialIODeviceMultiplexer
{
    Q_OBJECT
public:
    explicit QMultiPortMultiplexer( QSerialIODevice *device, QObject *parent = 0 );
    ~QMultiPortMultiplexer();

    bool addChannel( const QString& name, QSerialIODevice *device );
    QSerialIODevice *channel( const QString& name );

private:
    QMultiPortMultiplexerPrivate *d;
};

#endif /* QMULTIPORTMULTIPLEXER_H */
