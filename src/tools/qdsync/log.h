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
#ifndef LOG_H
#define LOG_H

#include <qtopialog.h>
#include "trace.h"

#include <QTextBrowser>
extern QTextBrowser *qdsync_tb;
#define USERLOG(x)\
do {\
    if ( qdsync_tb ) {\
        LOG() << x;\
        qdsync_tb->append(x);\
    }\
} while ( 0 )

#endif
