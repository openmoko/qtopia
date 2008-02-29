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

#include "launcher.h"

#include <QCoreApplication>

#include <stdio.h>

void print_help(const char *app)
{
    fprintf(stdout, "Useage: %s <application>\n", app);
    fprintf(stdout, "The %s will send a message to the server and wait until\n", app);
    fprintf(stdout, "the application starts...\n");
}

int main( int argc, char *argv[] )
{
    if (argc != 2) {
        print_help(argv[0]);
        return -1;
    }

    QCoreApplication app(argc, argv);

    Launcher launcher;
    launcher.startup(argv[1]);

    app.exec();
}
