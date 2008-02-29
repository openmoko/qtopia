/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
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
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef DIALUPSETTINGS_H
#define DIALUPSETTINGS_H

#include <qpe/networkinterface.h>
#include "dialupbase.h"

class ProxiesPage;

class Dialup : public DialupBase 
{
public:
    Dialup( QWidget* parent );

    ProxiesPage *proxies;
};

class DialupImpl : public NetworkInterface
{
public:
    DialupImpl();
    virtual ~DialupImpl();

#ifndef QT_NO_COMPONENT
    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT
#endif

    virtual bool doProperties( QWidget *parent, Config& );
    virtual bool create( Config& );
    virtual bool remove( Config& );
    virtual bool start( Config& );
    virtual bool start( Config&, const QString& );
    virtual bool stop( Config& );
    virtual bool needPassword( Config& ) const;

private:
    ulong ref;
};

#endif

