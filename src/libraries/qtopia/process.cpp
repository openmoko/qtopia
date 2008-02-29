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

#include <stdio.h>
#include <stdlib.h>

#include "process.h"

#ifndef QT_NO_PROCESS

#include "qapplication.h"


Process::Process( QObject *parent, const char *name )
    : QObject( parent, name ), ioRedirection( FALSE ), notifyOnExit( FALSE ),
    wroteToStdinConnected( FALSE )
{
    init();
}

Process::Process( const QString& arg0, QObject *parent, const char *name )
    : QObject( parent, name ), ioRedirection( FALSE ), notifyOnExit( FALSE ),
    wroteToStdinConnected( FALSE )
{
    init();
    addArgument( arg0 );
}

Process::Process( const QStringList& args, QObject *parent, const char *name )
    : QObject( parent, name ), ioRedirection( FALSE ), notifyOnExit( FALSE ),
    wroteToStdinConnected( FALSE )
{
    init();
    setArguments( args );
}


QStringList Process::arguments() const
{
    return _arguments;
}

void Process::setArguments( const QStringList& args )
{
    _arguments = args;
}

void Process::addArgument( const QString& arg )
{
    _arguments.append( arg );
}

bool Process::exec( const QString& in, QString& out, QStringList *env )
{
    QByteArray sout;
    QByteArray sin = in.local8Bit(); sin.resize(sin.size()-1); // cut nul
    bool r = exec( sin, sout, env );
    out = QString::fromLocal8Bit(sout);
    return r;
}

#endif
