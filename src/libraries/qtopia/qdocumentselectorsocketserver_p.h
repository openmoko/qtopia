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
#ifndef QDOCUMENTSELECTORSOCKETSERVER_P_H
#define QDOCUMENTSELECTORSOCKETSERVER_P_H

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

#include <qtopia/private/qdocumentserverchannel_p.h>
#include <private/qunixsocketserver_p.h>
#include <QContent>

class QDocumentSelector;
class QDialog;
class NewDocumentDialog;
class SaveDocumentDialog;

class QDocumentSelectorServer : public QDocumentServerHost
{
    Q_OBJECT
public:
    QDocumentSelectorServer( QObject *parent = 0 );
    virtual ~QDocumentSelectorServer();

protected:
    virtual QDocumentServerMessage invokeMethod( const QDocumentServerMessage &message );
    virtual void invokeSlot( const QDocumentServerMessage &message );

private slots:
    void documentSelected( const QContent &document );
    void newDocumentAccepted();
    void saveDocumentAccepted();
    void rejected();

private:
    QDocumentSelector *m_selector;
    QDialog *m_selectorDialog;

    NewDocumentDialog *m_newDocumentDialog;
    SaveDocumentDialog *m_saveDocumentDialog;

    QIODevice::OpenMode m_openMode;
    QContent m_selectedDocument;
};

class QDocumentSelectorSocketServer : public QUnixSocketServer
{
public:
    QDocumentSelectorSocketServer( QObject *parent = 0 );

protected:
    virtual void incomingConnection( int socketDescriptor );
};

Q_DECLARE_USER_METATYPE_ENUM(QIODevice::OpenMode);

#endif
