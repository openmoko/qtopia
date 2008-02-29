/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include "kernelkeymapkbddriverplugin.h"
#include "kernelkeymapkbdhandler.h"

#include <qtopiaglobal.h>
#include <qtopialog.h>

KernelkeymapKbdDriverPlugin::KernelkeymapKbdDriverPlugin( QObject *parent )
    : QKbdDriverPlugin( parent )
{
}

KernelkeymapKbdDriverPlugin::~KernelkeymapKbdDriverPlugin()
{
}

QWSKeyboardHandler* KernelkeymapKbdDriverPlugin::create(const QString& driver, const QString&)
{
    qLog(Input) << "KernelkeymapKbdDriverPlugin:create()";
    return create( driver );
}

QWSKeyboardHandler* KernelkeymapKbdDriverPlugin::create( const QString& driver)
{
    if( driver.toLower() == "kernelkeymapkbdhandler" ) {

        return new KernelkeymapKbdHandler();
    }
    return 0;
}

QStringList KernelkeymapKbdDriverPlugin::keys() const
{
    return QStringList() << "kernelkeymapkbdhandler";
}

QTOPIA_EXPORT_QT_PLUGIN(KernelkeymapKbdDriverPlugin)
