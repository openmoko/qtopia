/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef QRECORDBASEDSIMFILE_H
#define QRECORDBASEDSIMFILE_H

#include <qtelephonynamespace.h>
#include <qtopiaglobal.h>

class QRecordBasedSimFilePrivate;

class QTOPIAPHONE_EXPORT QRecordBasedSimFile : public QObject
{
    Q_OBJECT
public:
    explicit QRecordBasedSimFile( const QString& fileid,
                                  const QString& service = QString(),
                                  QObject *parent = 0 );
    ~QRecordBasedSimFile();

    void requestFileInfo();
    void read( int recno, int recordSize = -1 );
    void write( int recno, const char *data, int len );
    void write( int recno, const QByteArray& data );

signals:
    void error( QTelephony::SimFileError err );
    void fileInfo( int numRecords, int recordSize );
    void readDone( const QByteArray& data, int recno );
    void writeDone( int recno );

private slots:
    void serviceUnavailable();
    void serverError( const QString& reqid, QTelephony::SimFileError err );
    void serverFileInfo( const QString& reqid, int size, int recordSize );
    void serverReadDone( const QString& reqid, const QByteArray& data, int pos );
    void serverWriteDone( const QString& reqid, int pos );

private:
    QRecordBasedSimFilePrivate *d;
};

#endif /* QRECORDBASEDSIMFILE_H */
