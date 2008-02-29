/****************************************************************************
** $Id: qt/src/kernel/qsoundqss_qws.h   2.3.12   edited 2005-10-27 $
**
** Definition of Qt Sound System
**
** Created : 001017
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QSOUNDQSS_H
#define QSOUNDQSS_H

#ifndef QT_H
#include <qserversocket.h>
#endif // QT_H
#include "qwssocket_qws.h"

#if defined(QT_NO_NETWORK) || defined(QT_NO_DNS)
#define QT_NO_SOUNDSERVER
#endif

#ifndef QT_NO_SOUND

class QWSSoundServerData;

class QWSSoundServer : public QObject {
    Q_OBJECT
public:
    QWSSoundServer(QObject* parent=0);
    ~QWSSoundServer();
    void setSystemVolume( int );
    void playFile( const QString& filename , int );
    void stopFile( int );
    void pauseFile( int );
    void resumeFile( int );

private:
    QWSSoundServerData* d;
};

#ifndef QT_NO_SOUNDSERVER

// Only when managed
enum QSSMessage {
    QSS_Request,
    QSS_Release,
    QSS_Ack,
    QSS_Nak,
    QSS_Deferred
};

class QWSSoundClient : public QWSSocket {
    Q_OBJECT
public:

    enum SoundFlags {
	Priority = 0x01,
	Streaming = 0x02  // currently ignored, but but could set up so both Raw and non raw can be done streaming or not.
    };
    enum DeviceErrors {
	ErrOpeningAudioDevice = 0x01,
	ErrOpeningFile = 0x02,
	ErrReadingFile = 0x04
    };
    
    QWSSoundClient( QObject* parent=0 );
    ~QWSSoundClient( );
    void reconnect();
    void play( int id, const QString& filename );
    void play( int id, const QString& filename, int volume, int flags = 0 );
    void playRaw( int id, const QString&, int, int, int, int flags = 0 );

    void pause( int id );
    void stop( int id );
    void resume( int id );
    void setVolume( int id, int left, int right );
    void setSystemVolume( int v );
    void setMute( int id, bool m );

    // ### playPriorityOnly set at start of incoming call, unset at end of incoming call
    // to be used by server only, to protect phone conversation/rings.
    void playPriorityOnly(bool);
    
    // If silent, tell sound server to release audio device
    // Otherwise, allow sound server to regain audio device
    void setSilent( bool );
    
#ifdef QT_QSS_MANAGED_DSP
    void send( QSSMessage message ); // Only when managed 
#endif

signals:
    void soundCompleted(int);
    void deviceReady(int id);
    void deviceError(int id, QWSSoundClient::DeviceErrors);
    
    void received( QSSMessage message ); // Only when managed

private slots:
    void tryReadCommand();
    void emitConnectionRefused();
};

class QWSSoundServerSocket : public QWSServerSocket {
    Q_OBJECT

public:
    QWSSoundServerSocket(QObject* parent=0, const char* name=0);
    void newConnection(int s);

signals:
    void message( int, QSSMessage ); // Only when managed
    void received( int, QSSMessage ); // Only when managed

    void playFile(int, int, const QString&);
    void playFile(int, int, const QString&, int, int);
    void playRawFile(int, int, const QString&, int, int, int, int);
    void pauseFile(int, int);
    void stopFile(int, int);
    void resumeFile(int, int);
    void setVolume(int, int, int, int);
    void setSystemVolume(int);
    void setMute(int, int, bool);

    void stopAll(int);

    void playPriorityOnly(bool);
    
    void setSilent( bool );

    void soundFileCompleted(int, int);
    void deviceReady(int, int);
    void deviceError(int, int, int);
};
#endif

#endif // QT_NO_SOUND

#endif
