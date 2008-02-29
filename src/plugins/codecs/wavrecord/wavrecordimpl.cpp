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

#include "wavrecord.h"
#include "wavrecordimpl.h"


WavRecorderPluginImpl::WavRecorderPluginImpl()
    : ref(0)
{
}


WavRecorderPluginImpl::~WavRecorderPluginImpl()
{
}


#ifndef QT_NO_COMPONENT


QRESULT WavRecorderPluginImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( ( uuid == IID_QUnknown ) || ( uuid == IID_MediaRecorderPlugin ) ) {
	*iface = this, (*iface)->addRef();
	return QS_OK;
    } else {
	return QS_FALSE;
    }
}


Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( WavRecorderPluginImpl )
}


#endif


MediaRecorderEncoder *WavRecorderPluginImpl::encoder()
{
    return new WavRecorderPlugin;
}
