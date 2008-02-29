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

#include "sandboxedprocess.h"
#include <sys/resource.h>
#include <qtopianamespace.h>
#include <unistd.h>
#include <qsettings.h>


/*!
  \internal
  \class SandboxedProcess
  \brief The SandboxedProcess class is used to start untrusted applications
         under sandboxed conditions

  The level of restrictions are specified in Sandbox.conf.
  At present the only resources that are limited are the processes' total
  available memory and CPU time.

  The format of Sandbox.conf is as follows
  [Limits]
  AddressSpace=x
  CpuTime=y

  where x specifies the maximum size in bytes of the process' total available
  memory, and y specifies the maximum amount of CPU time in seconds.  If
  Sandbox.conf does not exist or values are not specified default values
  for memory and cputime are used.
*/

SandboxedProcess::SandboxedProcess( QObject *parent )
    : QProcess( parent )
{
}


/*! \internal */
void SandboxedProcess::setupChildProcess()
{
#if defined Q_OS_UNIX

    QSettings conf( "Trolltech", "Sandbox" );
    conf.beginGroup( "Limits" );

    QList<int> resources;
    QList<rlim_t> limits;

    //current default values are arbitrary
    resources << RLIMIT_AS;
    limits <<  conf.value( "AddressSpace", 5120000 ).toULongLong();

    resources << RLIMIT_CPU;
    limits << conf.value( "CpuTime", 5).toUInt();

    struct rlimit limit;
    for (int i = 0; i < resources.size(); i++ )
    {
        limit.rlim_cur = limits[i];
        limit.rlim_max = limits[i];
        setrlimit( resources[i], &limit );
    }

#endif
}
