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
#ifndef QIR_H
#define QIR_H

#include <qobject.h>

class QCopChannel;
class QIrServer;
class ReceiveDialog;
class SendDialog;

class QIr : public QObject
{
    Q_OBJECT
public:
    QIr( QObject *parent = 0, const char *name = 0 );
    ~QIr();

public slots:
    void receiving( int, const QString&, const QString &);
    void progress( int size );
    void obexMessage(const QCString&, const QByteArray&);

    void dialogDestroyed();
    void sendDialogDestroyed();
    void done();
    
private:
    QIrServer *obexServer;
    QCopChannel *obexChannel;
    ReceiveDialog *receiveDialog;
    SendDialog *sendDialog;
};

#endif
