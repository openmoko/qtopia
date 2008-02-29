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

#ifndef BSCIDRMAGENTSERVICE_H
#define BSCIDRMAGENTSERVICE_H

#include <qdrmcontentplugin.h>
#include <QtopiaAbstractService>
#include <stdlib.h>
#include <bsci.h>
#include <qtopiaglobal.h>
#include <QQueue>
#include <QPair>

class QDSActionRequest;
class BSciDrmAgentServicePrivate;

class BSciDrmAgentService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    BSciDrmAgentService( QObject *parent = 0 );
    virtual ~BSciDrmAgentService();

public slots:
    void handleProtectedRightsObject( const QDSActionRequest &request );
    void handleXmlRightsObject( const QDSActionRequest &request );
    void handleWbXmlRightsObject( const QDSActionRequest &request );
    void handleRoapTrigger( const QDSActionRequest &request );
    void handleRoapPdu( const QDSActionRequest &request );

    void convertMessage( const QDSActionRequest &request );;
private:
    void handleXmlRightsObject( const QByteArray &object );
    void handleWbXmlRightsObject( const QByteArray &object );
    void handleRoapTrigger( const QByteArray &trigger );
    void handleProtectedRightsObject( const QByteArray &object );
    void handleRoapPdu( const QByteArray &pdu );

    QByteArray convertMessage( const QByteArray &message );
};


#endif
