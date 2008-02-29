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
#ifndef TODO_PLUGIN_IMPL_H
#define TODO_PLUGIN_IMPL_H


#include <qtopia/todayplugininterface.h>

class TodoPlugin;

class QTOPIA_PLUGIN_EXPORT TodoPluginImpl : public TodayInterface
{
public:
    TodoPluginImpl();
    virtual ~TodoPluginImpl();

    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT

    virtual TodayPluginInterface* object(QObject *parent, const char *name);
    virtual QObject* objectFor(TodayPluginInterface *);

private:
    TodoPlugin *todoPlugin;
    ulong ref;
};


#endif

