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

#ifndef SANDBOXINSTALL_H
#define SANDBOXINSTALL_H

#include "installcontrol.h"

#include <qtopiasxe.h>

class SandboxUninstallJob; 
class SandboxInstallJob : public QObject
{
    Q_OBJECT
    friend class SandboxUninstallJob;
    public:
        SandboxInstallJob( const InstallControl::PackageInfo *, const QString &, ErrorReporter *errorReporter = 0 );
        virtual ~SandboxInstallJob() {};
        QString destinationPath() const { return destination; }
        bool isAborted() const { return abort; }
        bool preprocessPackageFiles( const QString &f = QString() );
        void removeDestination() const;
        bool installContent();
        bool setupSandbox();
        void runSandbox( const QString &, const QString & );
        bool createLink( const QString &target, const QString &link );

#ifndef QT_NO_SXE
    signals:
        void newBinary(SxeProgramInfo &pi);
#endif

    private:
        void mediaSandboxRoot();
        void registerHelpFiles( const QString& helpDir );
        void registerI18NFiles( const QString& i18nDir );
        bool setupSettingsFiles();
        void applyDomainRules( const QString &, const QString & );
        const InstallControl::PackageInfo *package;
        QString destination;
        QString media;
        QStringList desktopPaths;
        bool abort;
        ErrorReporter *reporter;

        static void reloadRules();
};

class SandboxUninstallJob
{
    public:
        SandboxUninstallJob( const InstallControl::PackageInfo *, const QString &media, ErrorReporter *reporter = 0 );
        void terminateApps() const;
        void unregisterPackageFiles() const;
        void dismantleSandbox() const;
        void rollBackSandboxRule( const QString &binPath ) const;
        static void clearMiscFiles( const InstallControl::PackageInfo * );

    private:
        QStringList getPackageBinaryPaths( const QString &path = QString() ) const;
        void removePackage() const;
        static void clearMiscFilesHelper( const QDir &dir,  const InstallControl::PackageInfo *);

        const InstallControl::PackageInfo *package;
        QString packagePath;
        ErrorReporter *reporter;
};


#endif
