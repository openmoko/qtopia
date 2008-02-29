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

#ifndef SANDBOXINSTALL_H
#define SANDBOXINSTALL_H

#include "installcontrol.h"

#include <qtopiasxe.h>

class SandboxInstallJob : public QObject
{
    Q_OBJECT
    public:
        SandboxInstallJob( const InstallControl::PackageInfo *, const QString &, ErrorReporter *errorReporter = 0 );
        virtual ~SandboxInstallJob() {};
        QString destinationPath() const { return destination; }
        bool isAborted() const { return abort; }
        bool registerPackageFiles( const QString &f = QString() );
        void removeDestination() const;
        bool installContent();
        bool setupSandbox();
        void runSandbox( const QString & );
        bool createLink( const QString &target, const QString &link );

#ifndef QT_NO_SXE
    signals:
        void newBinary(SxeProgramInfo &pi);
#endif

    private:
        void mediaSandboxRoot();
        void clearMiscFiles() const;
        const InstallControl::PackageInfo *package;
        QString destination;
        QString media;
        QStringList desktopPaths;
        bool abort;
        ErrorReporter *reporter;
};

class SandboxUninstallJob
{
    public:
        SandboxUninstallJob( const InstallControl::PackageInfo *, const QString &media, ErrorReporter *reporter = 0 );
        void unregisterPackageFiles() const;
        void dismantleSandbox() const;
        void rollBackSandboxRule( const QString &binPath ) const;

    private:
        QStringList getPackageBinaryPaths( const QString &path = QString() ) const;
        void removePackage() const;

        const InstallControl::PackageInfo *package;
        QString packagePath;
};


#endif
