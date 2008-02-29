/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#ifndef DIALERSERVICE_H
#define DIALERSERVICE_H

#include <qtopiaabstractservice.h>
class QUniqueId;
class DialerService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    DialerService( QObject *parent )
        : QtopiaAbstractService( "Dialer", parent )
        { publishAll(); }

public:
    ~DialerService();

public slots:
    virtual void dialVoiceMail() = 0;
    virtual void dial( const QString& name, const QString& number ) = 0;
    virtual void dial( const QString& number, const QUniqueId& contact ) = 0;
    virtual void showDialer( const QString& digits ) = 0;
};

#endif
