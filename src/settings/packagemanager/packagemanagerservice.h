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

#ifndef PACKAGEMANAGERSERVICE_H
#define PACKAGEMANAGERSERVICE_H

#include <QHttp>
#include <QBuffer>
#include <QFile>
#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QtopiaAbstractService>

#include "installcontrol.h"

class PackageServiceInstaller;
class PackageView;
class QDSActionRequest;

class PackageManagerService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    PackageManagerService( PackageView *parent );

public slots:
    void installPackageConfirm( const QString &url );
    void installPackage( const QDSActionRequest &request );

private slots:
    void installFinished();
private:

    PackageServiceInstaller *m_installer;
    PackageView *m_packageView;
    QStringList m_pendingUrls;
    QStringList m_pendingDescriptors;
};

class ServicePackageDetails;

class PackageServiceInstaller : public QDialog, public ErrorReporter
{
    Q_OBJECT
public:
    PackageServiceInstaller( QWidget *parent = 0, Qt::WindowFlags flags = 0 );

    bool installActive() const{ return m_installActive; }

    void reportError( const QString &error );

public slots:
    void installPackage( const QString &url );
    void installPackage( const QByteArray &descriptor );

    virtual void accept();
    virtual void reject();

private slots:
    void confirmInstall( InstallControl::PackageInfo package );
    void installPendingPackage();
    void headerDownloadDone( bool error );
    void packageDownloadDone( bool error );

    void updateProgress( int done, int total );

private:
    QHttp m_http;
    QBuffer m_headerBuffer;
    QFile m_packageFile;
    InstallControl::PackageInfo m_pendingPackage;
    InstallControl m_installer;

    QLabel *m_progressLabel;
    QProgressBar *m_progressBar;
    ServicePackageDetails *m_packageDetails;
    bool m_installActive;
};

#endif
