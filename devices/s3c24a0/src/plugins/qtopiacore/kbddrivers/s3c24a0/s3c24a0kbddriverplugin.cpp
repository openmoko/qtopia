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

#include "s3c24a0kbddriverplugin.h"
#include "s3c24a0kbdhandler.h"

#include <qtopiaglobal.h>

S3c24a0KbdDriverPlugin::S3c24a0KbdDriverPlugin( QObject *parent )
    : QKbdDriverPlugin( parent )
{}

S3c24a0KbdDriverPlugin::~S3c24a0KbdDriverPlugin()
{}

QWSKeyboardHandler* S3c24a0KbdDriverPlugin::create(const QString& driver, const QString&)
{
    qWarning("S3c24a0KbdDriverPlugin:create()");
    return create( driver );
}

QWSKeyboardHandler* S3c24a0KbdDriverPlugin::create( const QString& driver)
{
    if( driver.toLower() == "s3c24a0kbdhandler" ) {
        qWarning("Before call S3c24a0KbdHandler()");
        return new S3c24a0KbdHandler();
    }
    return 0;
}

QStringList S3c24a0KbdDriverPlugin::keys() const
{
    return QStringList() << "s3c24a0kbdhandler";
}

QTOPIA_EXPORT_QT_PLUGIN(S3c24a0KbdDriverPlugin)
