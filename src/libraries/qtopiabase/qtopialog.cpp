/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include <qtopialog.h>
#include <QString>
#include <QSettings>

Q_GLOBAL_STATIC_WITH_ARGS(QSettings, logSettings, ("Trolltech", "Log"));

bool qtopiaLogRequested( const char* category )
{
    return logSettings()->value(QLatin1String(category)+"/Enabled",0).toBool();
}

bool qtopiaLogEnabled( const char* category )
{
#undef QTOPIA_LOG_OPTION
#undef QLOG_DISABLE
#undef QLOG_ENABLE
#define QTOPIA_LOG_OPTION(dbgcat)
#define QLOG_DISABLE(dbgcat) if ( strcmp(category,#dbgcat)==0 ) return 0;
#define QLOG_ENABLE(dbgcat) if ( strcmp(category,#dbgcat)==0 ) return 1;
#include <qtopialog-config.h>
    return qtopiaLogRequested(category);
}

bool qtopiaLogOptional( const char* category )
{
#undef QTOPIA_LOG_OPTION
#undef QLOG_DISABLE
#undef QLOG_ENABLE
#define QTOPIA_LOG_OPTION(dbgcat) if ( strcmp(category,#dbgcat)==0 ) return 1;
#define QLOG_DISABLE(dbgcat)
#define QLOG_ENABLE(dbgcat)
#include <qtopialog-config.h>
    return 0;
}

