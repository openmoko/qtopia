/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include  <qobject.h>

class QIrPrivate;

class QIrServer : public QObject
{
  Q_OBJECT
public:
    QIrServer( QObject *parent = 0, const char *name = 0 );
    ~QIrServer();

    bool isBeaming() const { return bip; }
    bool isReceiving() const { return rip; }

    static QString landingZone( bool inbound=true );

public slots:

    void beam( const QString& filename, const QString& mimetype );
    void cancelBeam();

    void setReceivingEnabled( bool );
    void receiving(bool);
    void cancelReceive();

signals:
    void beamDone();
    void beamError();

    void fileComplete();
    void receiveInit();
    void receiving( int size, const QString& filename, const QString& mime );
    void receiveDone();
    void receiveError();
    
    void progressSend( int size );
    void progressReceive( int size );
    void statusMsg(const QString &);
    
    void abortBeam(); //private
    void abortReceive(); //private

private slots:
    void rError();
    void bDone();
    void bError();
    void clean();

private:
    QIrPrivate *data;
    bool bip, rip;
    bool needClean;
};
