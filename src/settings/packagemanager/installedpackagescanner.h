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

#ifndef INSTALLEDPACKAGESCANNER_H
#define INSTALLEDPACKAGESCANNER_H

#include <QThread>
#include <QStringList>

#include "installcontrol.h"

class AbstractPackageController;
class QEventLoop;

class InstalledPackageScanner : public QThread
{
    Q_OBJECT
public:
    static const int initProgress;
    static const int maxProgress;

    InstalledPackageScanner( QObject *parent );
    virtual ~InstalledPackageScanner();
    virtual void run();
    void setLocations( const QStringList & );
    bool wasAborted() const { return aborted; }
    QString getError() const { return error; }
public slots:
    void cancel();
signals:
    void progressValue( int );
    void newPackage( InstallControl::PackageInfo * );
private:
    void scan( const QString & );
    AbstractPackageController *pkgController;
    QEventLoop *eventLoop;
    QStringList locations;
    bool aborted;
    QString error;
};

#endif
