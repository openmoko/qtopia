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

#include "sxemonitor.h"
#include "sxemonqlog.h"
#include <qtopiaapplication.h>

QSXE_APP_KEY
int main(int argc, char* argv[])
{
    QSXE_SET_APP_KEY(argv[0])

    QtopiaApplication* app = new QtopiaApplication(argc, argv);
    app->registerRunningTask("sxemonitor");
    SxeMonitor *monitor = new SxeMonitor();
    Q_UNUSED( monitor );
    
    int returnVal = app->exec();
    return returnVal;
}
