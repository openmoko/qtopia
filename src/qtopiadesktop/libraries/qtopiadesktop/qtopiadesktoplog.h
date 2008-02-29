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
#ifndef QTOPIADESKTOPLOG_H
#define QTOPIADESKTOPLOG_H

#include <qdglobal.h>
#include <qlog.h>
#include <trace.h>

QD_EXPORT bool qtopiadesktopLogEnabled( const char *category );

#define QD_LOG_OPTION(x) QLOG_OPTION_VOLATILE(x,qtopiadesktopLogEnabled(#x))\
                         TRACE_OPTION(x,qtopiadesktopLogEnabled(#x))

//QLOG_UNCATEGORIZED()     // uncategorized logging
QD_LOG_OPTION(QDA)       // This is reserved for QtopiaDesktopApplication
QD_LOG_OPTION(PM)        // Plugin Manager logging
QD_LOG_OPTION(I18N)      // I18n-related stuff
QD_LOG_OPTION(UI)        // General UI-related stuff
QD_LOG_OPTION(UI_tray)   // System Tray

// "Generic" plugin LOG (by class)
QD_LOG_OPTION(QDApp)
QD_LOG_OPTION(QDLink)
QD_LOG_OPTION(QDCon)
QD_LOG_OPTION(QDDev)
QD_LOG_OPTION(QDSync)

// For uncategorized tracing
QD_LOG_OPTION(TRACE)

#endif
