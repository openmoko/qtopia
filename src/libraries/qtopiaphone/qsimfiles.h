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

#ifndef QSIMFILES_H
#define QSIMFILES_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>

class QTOPIAPHONE_EXPORT QSimFiles : public QCommInterface
{
    Q_OBJECT
public:
    explicit QSimFiles( const QString& service = QString(), QObject *parent = 0,
                        QCommInterface::Mode mode = Client );
    ~QSimFiles();

public slots:
    virtual void requestFileInfo( const QString& reqid, const QString& fileid );
    virtual void readBinary( const QString& reqid, const QString& fileid,
                             int pos, int len );
    virtual void writeBinary( const QString& reqid, const QString& fileid,
                              int pos, const QByteArray& data );
    virtual void readRecord( const QString& reqid, const QString& fileid,
                             int recno, int recordSize = -1 );
    virtual void writeRecord( const QString& reqid, const QString& fileid,
                              int recno, const QByteArray& data );

signals:
    void error( const QString& reqid, QTelephony::SimFileError err );
    void fileInfo( const QString& reqid, int size, int recordSize,
                   QTelephony::SimFileType type );
    void readDone( const QString& reqid, const QByteArray& data, int pos );
    void writeDone( const QString& reqid, int pos );
};

#endif /* QSIMFILES_H */
