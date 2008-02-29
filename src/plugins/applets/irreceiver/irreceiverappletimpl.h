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
#ifndef CLIPBOARDAPPLETIMPL_H
#define CLIPBOARDAPPLETIMPL_H

#include <qtopia/taskbarappletinterface.h>

class IRReceiverApplet;

class QTOPIA_PLUGIN_EXPORT IRReceiverAppletImpl : public TaskbarNamedAppletInterface
{
public:
    IRReceiverAppletImpl();
    virtual ~IRReceiverAppletImpl();

    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT

    virtual QWidget *applet( QWidget *parent );
    virtual int position() const;
    virtual QString name() const;
    virtual QPixmap icon() const;

private:
    IRReceiverApplet *irreceiver;
    QPixmap *icn;
    ulong ref;
};

#endif
