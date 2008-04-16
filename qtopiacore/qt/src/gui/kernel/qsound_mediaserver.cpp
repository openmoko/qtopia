/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.1, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qapplication.h"

#ifndef QT_NO_SOUND

#include "qsound.h"
#include "qpaintdevice.h"
#include "qsound_p.h"

#include "qhash.h"
#include "qfileinfo.h"

#include "qbytearray.h"
#include "qbuffer.h"
#include "quuid.h"
#include "qdatastream.h"
#include "qtopiaservices.h"

#include "qcopenvelope_p.h"

#define MEDIA_SERVICE "QPE/MediaServer"


class QAuServerMediaServer;

class QAuBucketMediaServer : public QObject, public QAuBucket
{
    Q_OBJECT
public:
    QAuBucketMediaServer( QAuServerMediaServer*, QSound*, QObject* parent = 0 );

    ~QAuBucketMediaServer();

    int id() const { return id_; }
    QSound* sound() const { return sound_; }
    void play();
    void stop();

signals:
    void done( QAuBucketMediaServer* );

private slots:
    void processMessage( const QString& msg, const QByteArray& data );

private:
    QCopChannel *m_channel;
    QUuid m_id;
    int id_;
    QSound *sound_;
    QAuServerMediaServer *server_;

    static int next;
};

int QAuBucketMediaServer::next = 0;

class QAuServerMediaServer : public QAuServer
{
    Q_OBJECT
public:
    QAuServerMediaServer( QObject* parent );

    void init( QSound* s )
    {
        QAuBucketMediaServer *bucket = new QAuBucketMediaServer( this, s );
        connect( bucket, SIGNAL(done(QAuBucketMediaServer*)),
            this, SLOT(complete(QAuBucketMediaServer*)) );
        setBucket( s, bucket );
    }

    void play( QSound* s )
    {
        bucket( s )->play();
    }

    void stop( QSound* s )
    {
        bucket( s )->stop();
    }

    bool okay() { return true; }

private slots:
    // Only for Media Server
    void complete( QAuBucketMediaServer* bucket )
    {
        QSound *sound = bucket->sound();
        if( decLoop( sound ) ) {
            play( sound );
        }
    }

protected:
    QAuBucketMediaServer* bucket( QSound *s )
    {
        return (QAuBucketMediaServer*)QAuServer::bucket( s );
    }

private:
};

QAuServerMediaServer::QAuServerMediaServer(QObject* parent) :
    QAuServer(parent)
{
}

QAuBucketMediaServer::QAuBucketMediaServer( QAuServerMediaServer *server, QSound *sound, QObject* parent )
    : QObject( parent ), sound_( sound ), server_( server )
{

    m_id = QUuid::createUuid();

    sound->setObjectName( m_id.toString() );

    m_channel = new QCopChannel( QString( "QPE/QSound/" ).append( m_id ), this );
    connect( m_channel, SIGNAL(received(QString,QByteArray)),
        this, SLOT(processMessage(QString,QByteArray)) );

    {
        QCopEnvelope envelope( MEDIA_SERVICE, "subscribe(QUuid)" );
        envelope << m_id;
    }

    {
        QString filepath = QFileInfo( sound_->fileName() ).absoluteFilePath();
        QCopEnvelope envelope( MEDIA_SERVICE, "open(QUuid,QString)" );
        envelope << m_id << filepath;
    }
}

void QAuBucketMediaServer::play()
{
    QCopEnvelope envelope( MEDIA_SERVICE, "play(QUuid)" );
    envelope << m_id;
}

void QAuBucketMediaServer::stop()
{
    QCopEnvelope envelope( MEDIA_SERVICE, "stop(QUuid)" );
    envelope << m_id;
}

void QAuBucketMediaServer::processMessage( const QString& msg, const QByteArray& data )
{
    Q_UNUSED(data);
    if( msg == "done()" ) {
        emit done( this );
    }
}

QAuBucketMediaServer::~QAuBucketMediaServer()
{
    QCopEnvelope envelope( MEDIA_SERVICE, "revoke(QUuid)" );
    envelope << m_id;
}


__attribute__((visibility("default"))) QAuServer* qt_new_audio_server_media_server()
{
    return new QAuServerMediaServer(qApp);
}

#include "qsound_mediaserver.moc"

#endif // QT_NO_SOUND
