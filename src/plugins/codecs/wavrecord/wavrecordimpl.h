/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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
#ifndef WAV_RECORD_IMPL_H
#define WAV_RECORD_IMPL_H


#include <mediarecorderplugininterface.h>


class WavRecorderPlugin;


class WavRecorderPluginImpl : public MediaRecorderCodecPlugin
{
public:
    WavRecorderPluginImpl();
    virtual ~WavRecorderPluginImpl();

    QStringList keys() const;

    virtual MediaRecorderEncoder *encoder();
};

QTOPIA_EXPORT_PLUGIN( WavRecorderPluginImpl )

#endif

