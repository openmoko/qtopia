/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
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

    enum State { Ready, Beaming, Receiving, Error };

    State state() const { return _state; }

public slots:

    void beam( const QString& filename, const QString& mimetype );
    void cancel();

    void setReceivingEnabled( bool );
    void receiving(bool);

signals:
    void beamDone();
    void beamError();

    void receiveInit();
    void receiving( int size, const QString& filename, const QString& mime );
    void received( const QString& filename, const QString& mime );
    void receiveError();
    
    void progress( int size );
    void statusMsg(const QString &);
    
    void abort(); //private

private slots:
    void mDone();
    void mError();

private:
    QIrPrivate *data;
    State _state;
};
