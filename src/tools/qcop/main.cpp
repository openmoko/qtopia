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

#include "qcopimpl.h"

#include <qtopiasxe.h>

#ifdef SINGLE_EXEC
#include <QtopiaApplication>
QTOPIA_ADD_APPLICATION(QTOPIA_TARGET,qcop)
#define MAIN_FUNC main_qcop
#else
#define MAIN_FUNC main
#endif

int doqcopimpl(int argc, char **argv);

QSXE_APP_KEY
int MAIN_FUNC(int argc, char **argv)
{
    QSXE_SET_APP_KEY(argv[0])

    return doqcopimpl(argc, argv);
}

