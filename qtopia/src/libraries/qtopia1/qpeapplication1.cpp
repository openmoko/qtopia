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

#include <qtopia/qpeapplication.h>

#include <sys/types.h>

#ifndef QT_NO_COP
#include <qtopia/qcopenvelope_qws.h>
#endif

#ifndef Q_OS_WIN32
#include <unistd.h>
#else
#include <process.h>
#endif

#include "qpe_show_dialog.cpp"

void QPEApplication::showDialog( QDialog* d, bool nomax )
{
    qpe_show_dialog( d, nomax );
}

int QPEApplication::execDialog( QDialog* d, bool nomax )
{
    showDialog(d,nomax);
    return d->exec();
}

/*!

  This method temporarily overrides the current global screen saver with
  the screenSaverHint \a hint, allowing applications to control screensaver
  activation during their execution.

  First availability: Qtopia 1.6

  \sa screenSaverHint
*/

void QPEApplication::setTempScreenSaverMode(screenSaverHint hint)
{
#ifndef QT_NO_COP
    int pid = ::getpid();
    QCopEnvelope("QPE/System", "setTempScreenSaverMode(int,int)") << hint << pid;
#endif
}

#ifdef Q_OS_WIN32
/*
  Provides minimal support for setting environment variables
 */
int setenv(const char* name, const char* value, int overwrite)
{
  QString msg(name);
  msg.append("=").append(value);
  // put env requires ownership of the value and will free it
  char * envValue = strdup(msg.local8Bit());
  return _putenv(envValue);
}

/*
  Provides minimal support for clearing environment variables
 */
void unsetenv(const char *name)
{
  setenv(name, "", 1);
}
#endif
