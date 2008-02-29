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

#include <qsoundqss_qws.h>
#include <qtopiaapplication.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#ifdef SINGLE_EXEC
QTOPIA_ADD_APPLICATION(QTOPIA_TARGET,mediaserver)
#define MAIN_FUNC main_mediaserver
#else
#define MAIN_FUNC main
#endif

QSXE_APP_KEY

int MAIN_FUNC(int argc, char **argv)
{
    QSXE_SET_APP_KEY(argv[0])

    QApplication a(argc, argv);
    (void)new QWSSoundServer(0);

    //if (1) {
    // later, check if root first
        setpriority(PRIO_PROCESS, 0, -15);
    //}
    // hook up some quit mech.
    return a.exec();
}

