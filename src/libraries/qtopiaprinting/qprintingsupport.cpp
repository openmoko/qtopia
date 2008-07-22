/***************************************************************************
**
** Copyright (C) 2008 Openmoko Inc.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2 or at your option any later version.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************/

#include "qprintingsupport.h"

#include "qtopiaservices.h"
#include <QPluginManager>
#include <QStringList>

namespace QPrintingSupport {

/*!
 This can be used to determine if the system has the runtime support for
 printing. This means at least one plugin exists that can carry out the print
 job.
 Depending on the policy this method might return true even if no plugins are
 currently installed because the printserver could interact with the installer
 to install printing.
 */
QTOPIAPRINTING_EXPORT bool hasPrintingSupport()
{
    if (QtopiaService::apps("Print").isEmpty())
        return false;

    QPluginManager manager("qtopiaprinting");
    return !manager.list().isEmpty();
}

}

