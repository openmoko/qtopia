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
#include <qtopiadesktoplog.h>
#include <desktopsettings.h>

bool qtopiadesktopLogEnabled( const char *_category )
{
    static QMap<QString,bool> *cache = 0;
    if ( cache && _category == 0 ) {
        delete cache;
        cache = 0;
        return false;
    }
    if ( !cache ) {
        cache = new QMap<QString,bool>;
    }
    QLatin1String category( _category );
    if ( cache->contains(category) )
        return (*cache)[category];

    DesktopSettings settings( "Log" );
    QVariant v = settings.value( category );
    if ( v.isNull() ) {
        v = DesktopSettings::debugMode()?1:0;
        if ( category == "TRACE" )
            v = 0;
        settings.setValue( category, v );
    }
    bool ret = v.toBool();
    (*cache)[QString(category)] = ret;
    //qDebug() << "category" << category << ret;
    return ret;
}

/*!
  \headerfile <qtopiadesktoplog.h>
  \title <qtopiadesktoplog.h>
  \ingroup headers
  \brief The <qtopiadesktoplog.h> header contains the category definitions used in Qtopia Sync Agent.

  The <qtopiadesktoplog.h> header contains the category definitions used in Qtopia Sync Agent.

  \quotefromfile libraries/qtopiadesktop/qtopiadesktoplog.h
  \skipto QLOG_UNCATEGORIZED
  \printuntil QDSync

  \sa <qlog.h>
*/

/*!
  \macro QDLOG_OPTION(CATEGORY)
  \relates <qtopiadesktoplog.h>

  Register a category for conditional logging. This enables qLog(\a{CATEGORY}) but
  the log messages are not displayed unless \a CATEGORY is enabled. This macro uses the
  qtopiadesktopLogEnabled() function to check if \a CATEGORY is enabled.
*/

/*!
  \fn bool qtopiadesktopLogEnabled( const char *category )
  \relates <qtopiadesktoplog.h>
  This function returns true if \a category has been enabled.

  \quotefromfile libraries/qtopiadesktop/qtopiadesktoplog.cpp
  \skipto DesktopSettings
  \printuntil return
*/
