/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qtopia/global.h>
#include <qtopia/config.h>
#include <qtopia/qpeapplication.h>
#include <qaccel.h>
#include <qarray.h>
#ifdef Q_OS_WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/*!
  \fn Global::mousePreferred()
  
  Returns TRUE if the application can expect the user to
  be able to easily produce \link QMouseEvent mouse events\endlink.

  Returns FALSE if the user \e cannot produce mouse events,
  or chooses not to (eg. does not plug in a mouse).

  Applications may use this to tune interactions.

  Note that you should only call this function after an instance of
  QPEApplication has been created. This function will return an undefined
  value if called prior to this.  

  First availability: Qtopia 2.0
*/

#ifdef QTOPIA_PHONE
extern bool qt_modalEditingEnabled;
#endif

bool Global::mousePreferred()
{
#ifdef QTOPIA_PHONE
    if( qt_modalEditingEnabled )
	return FALSE; //keypad phone
#endif
    // pda or touchscreen phone
    return TRUE;
}

/*!
  Returns TRUE if \a key is available on the device.  The buttons
  may be either fixed function buttons, such as Key_Menu, or user
  mappable buttons, such as Key_F1.

  First availability: Qtopia 2.0

  \sa ContextBar, DeviceButtonManager
*/
bool Global::hasKey(int key)
{
    static QArray<int> *buttons = 0;

    if (!buttons) {
	buttons = new QArray<int>;
	Config cfg(Global::defaultButtonsFile(), Config::File);
	cfg.setGroup("SystemButtons");
	if (cfg.hasKey("Count")) {
	    int count = cfg.readNumEntry("Count", 0);
	    if (count) {
		buttons->resize(count);
		for (int i = 0; i < count; i++) {
		    QString is = QString::number(i);
		    (*buttons)[i] = QAccel::stringToKey(cfg.readEntry("Key"+is));
		}
	    }
	}
    }

    return (*buttons).contains(key);
}

/*!
  System independant sleep. Sleep for \a secs seconds.

  First availability: Qtopia 2.0
*/
void Global::sleep( unsigned long secs )
{
#ifdef Q_OS_WIN32
    ::Sleep( secs * 1000 );
#else
    ::sleep( secs );
#endif
}

/*!
  System independant sleep. Sleep for \a msecs milliseconds.

  First availability: Qtopia 2.0
*/
void Global::msleep( unsigned long msecs )
{
#ifdef Q_OS_WIN32
    ::Sleep( msecs );
#else
    Global::usleep( msecs * 1000 );
#endif
}

/*!
  System independant sleep. Sleep for \a usecs microseconds.

  First availability: Qtopia 2.0
*/
void Global::usleep( unsigned long usecs )
{
#ifdef Q_OS_WIN32
    ::Sleep( ( usecs / 1000 ) + 1 );
#else
    if ( usecs >= 1000000 )
	::sleep( usecs / 1000000 );
    ::usleep( usecs % 1000000 );
#endif
}

#include "../qtopia/global_qtopiapaths.cpp"

/*!
  List of base directories containing Qtopia files.
  This is like QPEApplication::qpeDir(), but allows
  multiple directories to be specified.

  The value is set by the colon-seperated environment
  variable QPEDIRS. The default is just $QPEDIR.

  First availability: Qtopia 2.1
*/
QStringList Global::qtopiaPaths()
{
    return global_qtopiapaths();
}

#include "qpe_defaultButtonsFile.cpp"

/*!
  Return the name of the defaultbuttons.conf file.
  This allows Qtopia to use a QVFb-supplied defaultbuttons.conf file (if one exists).

  First availability: Qtopia 2.1
*/
QString Global::defaultButtonsFile()
{
    return ::qpe_defaultButtonsFile();
}

