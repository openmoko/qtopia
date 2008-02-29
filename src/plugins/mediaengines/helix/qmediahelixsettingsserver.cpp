/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#include <qstringlist.h>

#include "qmediahelixsettingsserver.h"

#include <config.h>
#include <hxcom.h>
#include <hxcore.h>
#include <hxccf.h>
#include <hxprefs.h>
#include <ihxpckts.h>

#include <qdebug.h>
#include <reporterror.h>

namespace qtopia_helix
{

class QMediaHelixSettingsServerPrivate
{
public:
    QStringList     options;
    IHXClientEngine *engine;

    QVariant value( QString const& name ) const;
    void setValue( QString const& name, QVariant const& vlaue );
};

QVariant QMediaHelixSettingsServerPrivate::value( QString const& name ) const
{
    QString s;

    IHXPreferences *preferences = 0;
    IHXBuffer *buffer = 0;

    if( engine->QueryInterface( IID_IHXPreferences, (void**)&preferences ) == HXR_OK ) {
        if( preferences->ReadPref(name.toLatin1().constData(), buffer ) == HXR_OK ) {
            s = (const char*)buffer->GetBuffer();
        }
    } else {
        REPORT_ERROR( ERR_UNSUPPORTED );
    }

    HX_RELEASE( buffer );
    HX_RELEASE( preferences );

    return s;
}

void QMediaHelixSettingsServerPrivate::setValue( QString const& name, QVariant const& value )
{
    IHXPreferences *preferences = 0;
    IHXCommonClassFactory *factory = 0;
    IHXBuffer *buffer = 0;

    if( engine->QueryInterface( IID_IHXPreferences, (void**)&preferences ) == HXR_OK &&
        engine->QueryInterface( IID_IHXCommonClassFactory, (void**)&factory ) == HXR_OK ) {
        factory->CreateInstance( CLSID_IHXBuffer, (void**)&buffer );

        QString s = value.toString();
        buffer->Set( (const UCHAR*)s.toLatin1().data(), s.length() );

        preferences->WritePref( name.toLatin1().constData(), buffer );
    } else {
        REPORT_ERROR( ERR_UNSUPPORTED );
    }

    HX_RELEASE( buffer );
    HX_RELEASE( factory );
    HX_RELEASE( preferences );
}


QMediaHelixSettingsServer::QMediaHelixSettingsServer(IHXClientEngine *engine):
    QAbstractIpcInterface("/Media/Control/Helix/",
                          "Settings",
                          "HelixGlobalSettings",
                          NULL,
                          QAbstractIpcInterface::Server),
    d(new QMediaHelixSettingsServerPrivate)
{
    d->options << QLatin1String("Bandwidth") << QLatin1String("ServerTimeOut") <<
                  QLatin1String("ConnectionTimeOut");

    d->engine = engine;

    setValue(QLatin1String("AvailableSettings"), d->options);

    setValue(QLatin1String("Bandwidth"), d->value(QLatin1String("Bandwidth")));
    setValue(QLatin1String("ServerTimeOut"), d->value(QLatin1String("ServerTimeOut")));
    setValue(QLatin1String("ConnectionTimeOut"), d->value(QLatin1String("ConnectionTimeOut")));

    proxyAll(*metaObject());
}

QMediaHelixSettingsServer::~QMediaHelixSettingsServer()
{
    delete d;
}


//public slots:
void QMediaHelixSettingsServer::setOption(QString const& name, QVariant const& value)
{
    if (d->options.contains(name))
    {
        d->setValue(name, value);       // to helix
        setValue(name, value);          // for clients

        emit optionChanged(name, value);
    }
}

}   // ns qtopia_helix

