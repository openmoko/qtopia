/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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

void QPEApplication::setTempScreenSaverMode(screenSaverHint hint)
{
#ifndef QT_NO_COP    
    int pid = ::getpid();
    QCopEnvelope("QPE/System", "setTempScreenSaverMode(int,int)") << hint << pid;
#endif
}

#ifdef Q_OS_WIN32
/*!
  Provide minimal support for setting environment variables
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
  Provide minimal support for clearing environment variables
 */
void unsetenv(const char *name)
{
  setenv(name, "", 1);
}
#endif
