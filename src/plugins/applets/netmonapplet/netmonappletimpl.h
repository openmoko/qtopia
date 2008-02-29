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
#ifndef NETMONAPPLETIMPL_H
#define NETMONAPPLETIMPL_H

#include <qtopia/qpeglobal.h>
#include <qtopia/taskbarappletinterface.h>

class NetMonitorApplet;

class QTOPIA_PLUGIN_EXPORT NetMonitorAppletImpl : public TaskbarAppletInterface
{
public:
    NetMonitorAppletImpl();
    virtual ~NetMonitorAppletImpl();

    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT

    virtual QWidget *applet( QWidget *parent );
    virtual int position() const;

private:
    NetMonitorApplet *netmon;
    ulong ref;
};

#endif
