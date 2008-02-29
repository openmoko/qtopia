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

#ifndef SANDBOXINSTALL_H
#define SANDBOXINSTALL_H

#include "installcontrol.h"

class SandboxInstallJob
{
    public:
        SandboxInstallJob( const InstallControl::PackageInfo *, const QString & );
        QString destinationPath() const { return destination; }
        bool isAborted() const { return abort; }
        void registerPackageFiles( const QString &f = QString() );
        void removeDestination() const;
        void installContent();
        void setupSandbox();
        void runSandbox( const QString & );
        bool createLink( const QString &target, const QString &link );
    private:
        void mediaSandboxRoot();
        const InstallControl::PackageInfo *package;
        QString destination;
        QString media;
        QStringList desktopPaths;
        bool abort;
};

#endif
