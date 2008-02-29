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

#ifndef QBINARYSIMFILE_H
#define QBINARYSIMFILE_H

#include <qtelephonynamespace.h>
#include <qtopiaglobal.h>

class QBinarySimFilePrivate;

class QTOPIAPHONE_EXPORT QBinarySimFile : public QObject
{
    Q_OBJECT
public:
    explicit QBinarySimFile( const QString& fileid, const QString& service = QString(),
                             QObject *parent = 0 );
    ~QBinarySimFile();

    void requestFileSize();
    void read( int pos, int len );
    void write( int pos, const char *data, int len );
    void write( int pos, const QByteArray& data );

signals:
    void error( QTelephony::SimFileError err );
    void fileSize( int size );
    void readDone( const QByteArray& data, int pos );
    void writeDone( int pos );

private slots:
    void serviceUnavailable();
    void serverError( const QString& reqid, QTelephony::SimFileError err );
    void serverFileInfo( const QString& reqid, int size );
    void serverReadDone( const QString& reqid, const QByteArray& data, int pos );
    void serverWriteDone( const QString& reqid, int pos );

private:
    QBinarySimFilePrivate *d;
};

#endif /* QBINARYSIMFILE_H */
