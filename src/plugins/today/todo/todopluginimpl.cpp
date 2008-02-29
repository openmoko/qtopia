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
#include "todoplugin.h"
#include "todopluginimpl.h"

TodoPluginImpl::TodoPluginImpl()
{
    todoPlugin = 0; 
}

TodoPluginImpl::~TodoPluginImpl()
{
}

TodayPluginInterface *TodoPluginImpl::object(QObject *parent, const char *name)
{
    if ( !todoPlugin )
	todoPlugin = new TodoPlugin(parent, name);

    return todoPlugin;
}

QObject* TodoPluginImpl::objectFor(TodayPluginInterface *task)
{
    return (TodoPlugin *) task;
}

QRESULT TodoPluginImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( ( uuid == IID_QUnknown ) || ( uuid == IID_TodayPlugin ) )
	*iface = this;
    else
	return QS_FALSE;

    (*iface)->addRef();
    return QS_OK;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( TodoPluginImpl );
}
