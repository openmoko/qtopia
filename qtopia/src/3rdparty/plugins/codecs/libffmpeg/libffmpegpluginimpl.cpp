/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
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
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "libffmpegplugin.h"
#include "libffmpegpluginimpl.h"


LibFFMpegPluginImpl::LibFFMpegPluginImpl() : libffmpegplugin(0), ref(0)
{
}


LibFFMpegPluginImpl::~LibFFMpegPluginImpl()
{
    if ( libffmpegplugin )
	delete libffmpegplugin;
}


MediaPlayerDecoder *LibFFMpegPluginImpl::decoder()
{
    if ( !libffmpegplugin )
	libffmpegplugin = new LibFFMpegPlugin;
    return libffmpegplugin;
}


MediaPlayerEncoder *LibFFMpegPluginImpl::encoder()
{
    return NULL;
}


#ifndef QT_NO_COMPONENT


QRESULT LibFFMpegPluginImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( ( uuid == IID_QUnknown )  || ( uuid == IID_MediaPlayerPlugin ) || ( uuid == IID_MediaPlayerPlugin_1_6 ) ) {
	*iface = this;
	(*iface)->addRef();
        return QS_OK;
    }
    return QS_FALSE;
}


Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( LibFFMpegPluginImpl )
}


#endif

