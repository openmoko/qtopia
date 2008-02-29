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

#ifndef QMODEMSIMFILES_P_H
#define QMODEMSIMFILES_P_H

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

#include <qmodemsimfiles.h>
#include <qatresult.h>

class QModemSimFileRequest : public QObject
{
    Q_OBJECT
public:
    QModemSimFileRequest
            ( QModemService *service, const QString& reqid,
              const QString& fileid, bool useCSIM, QObject *parent );
    ~QModemSimFileRequest();

    void chat( int cmd, int p1, int p2, int p3,
               const QString& extra = QString() );

private slots:
    void selectResult( bool ok, const QAtResult& result );
    void chatResult( bool ok, const QAtResult& result );

signals:
    void error( const QString& reqid, QTelephony::SimFileError err );

protected:
    QModemService *service;
    QString reqid;
    QString fileid;
    bool isWriting;
    bool retryFromRoot;
    bool retryRequested;
    bool selectFailed;
    bool errorReported;
    bool useCSIM;
    QString command;

    virtual void done( const QByteArray& data ) = 0;

private:
    void sendSelects( bool fromRoot );
    QString formatRequest( int cmd, const QString& fileid,
                           int p1, int p2, int p3,
                           const QString& extra = QString() );
};

class QModemSimFileInfoRequest : public QModemSimFileRequest
{
    Q_OBJECT
public:
    QModemSimFileInfoRequest
            ( QModemService *service, const QString& reqid,
              const QString& fileid, bool useCSIM, QObject *parent );
    ~QModemSimFileInfoRequest();

signals:
    void fileInfo( const QString& reqid, int size, int recordSize,
                   QTelephony::SimFileType type );

protected:
    void done( const QByteArray& data );
};

class QModemSimFileReadRequest : public QModemSimFileRequest
{
    Q_OBJECT
public:
    QModemSimFileReadRequest
            ( QModemService *service, const QString& reqid,
              const QString& fileid, int pos, bool useCSIM, QObject *parent );
    ~QModemSimFileReadRequest();

signals:
    void readDone( const QString& reqid, const QByteArray& data, int pos );

public slots:
    void fileInfo( const QString& reqid, int size, int recordSize );
    void infoError( const QString& reqid, QTelephony::SimFileError err );

protected:
    void done( const QByteArray& data );

private:
    int pos;
};

class QModemSimFileWriteRequest : public QModemSimFileRequest
{
    Q_OBJECT
public:
    QModemSimFileWriteRequest
            ( QModemService *service, const QString& reqid,
              const QString& fileid, int pos, bool useCSIM, QObject *parent );
    ~QModemSimFileWriteRequest();

signals:
    void writeDone( const QString& reqid, int pos );

protected:
    void done( const QByteArray& data );

private:
    int pos;
};

#endif /* QMODEMSIMFILES_P_H */
