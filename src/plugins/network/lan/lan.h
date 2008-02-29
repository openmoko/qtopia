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

#ifndef NETWORKSETTINGS_H
#define NETWORKSETTINGS_H

#include "lanbase.h"

#include <qtopia/networkinterface.h>

#include <qmap.h>

class QButtonGroup;
class ProxiesPage;


class Lan : public LanBase
{
    Q_OBJECT
public:
    Lan(Config&, QWidget* parent);
    static bool writeNetworkOpts( Config &config, QString scheme ="*" );

private slots:
    void ipSelect( int id );
    void chooseDefaultWepKey();
    void chooseWepType(int i);

private:
    void readConfig();
    bool writeConfig();
    virtual void accept();

private:
    Config& config;
    QButtonGroup *ipGroup;
    ProxiesPage *proxies;
};

class LanImpl : public NetworkInterface
{
public:
    LanImpl();
    virtual ~LanImpl();

#ifndef QT_NO_COMPONENT
    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT
#endif

    virtual bool doProperties( QWidget *parent, Config& );
    virtual bool create( Config& );
    virtual bool remove( Config& );
    virtual bool start( Config& );
    virtual bool stop( Config& );
    virtual QWidget* addStateWidget( QWidget*, Config& ) const;
    virtual bool isActive( Config& ) const;
    virtual bool isAvailable( Config& ) const;

private:
    ulong ref;
};

#endif
