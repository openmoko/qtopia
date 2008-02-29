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

#include "s3c24a0mousedriverplugin.h"
#include "s3c24a0mousehandler.h"

#include <qtopiaglobal.h>

S3c24a0MouseDriverPlugin::S3c24a0MouseDriverPlugin( QObject *parent )
    : QMouseDriverPlugin( parent )
{}

S3c24a0MouseDriverPlugin::~S3c24a0MouseDriverPlugin()
{}

QWSMouseHandler* S3c24a0MouseDriverPlugin::create(const QString& driver, const QString&)
{
    qWarning("S3c24a0MouseDriverPlugin:create()");
    return create( driver );
}

QWSMouseHandler* S3c24a0MouseDriverPlugin::create( const QString& driver)
{
    if( driver.toLower() == "s3c24a0mousehandler" ) {
        qWarning("Before call S3c24a0MouseHandler()");
        return new S3c24a0MouseHandler();
    }
    return 0;
}

QStringList S3c24a0MouseDriverPlugin::keys() const
{
    return QStringList() << "s3c24a0mousehandler" << "s3c24a0_ts";
}

QTOPIA_EXPORT_QT_PLUGIN(S3c24a0MouseDriverPlugin)
