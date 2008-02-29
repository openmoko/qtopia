/****************************************************************************
**
**Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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
#include <qtopiaapplication.h>
#include "sysmessages.h"

QSXE_APP_KEY
int main( int argc, char *argv[] )
{
    QSXE_SET_APP_KEY(argv[0]);
    QtopiaApplication app(argc, argv);
    new SysMessagesService( &app );
    return app.exec();
}
