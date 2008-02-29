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
#ifndef PACKAGEWIZARD_H
#define PACKAGEWIZARD_H
#include "pkwizard.h"
#include <qlistview.h>
#include <qlist.h>
#include <qdict.h>

class PackageItem;
class QProcess;
class DetailsPopup;

class PackageWizard : public PackageWizardBase
{ 
    Q_OBJECT

public:
    PackageWizard( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~PackageWizard();
    
public slots:
    void setDocument(const QString& fileref);

private slots:
    void showDetails();
    void newServer();
    void editServer(QListViewItem*);
    void removeServer();
    void nameChanged(const QString&);
    void urlChanged(const QString&);
    void updatePackageList();

    void infoReadyRead();
    void infoReadError();
    void infoDone();

    void setAppropriates();
    void updatePackageNext();
    void updateServerSelection();

private:
    bool eventFilter( QObject * o, QEvent *e );
    void done(int);

    void startRun();
    void endRun();
    void showPage( QWidget* );
    void startMultiRun(int jobs);
    bool runIpkg(const QStringList& args, QString& out);
    bool installIpkg( const QString &ipk, const QString &location, QString& out );
    QStringList linksInPackage(const QString& pkg, const QString &root );
    void revertFailedInstalls(QString& out);

    void showError(const QString& err);

    QProgressBar* progress();
    QProcess* infoProcess;
    QList<QListViewItem> infoPending;
    void startInfoProcess();
    QString info;

    void insertLocalPackageItems();
    void insertPackageItems(bool installed_only);
    QDict<void> installedPackages();

    //QString fullDetails(const QString& pk);
    //DetailsPopup* package_description;

    bool readIpkgConfig(const QString& conffile);

    QStringList findPackages( const QRegExp& re );
    void selectPackages( const QStringList& l );
    PackageItem* PackageWizard::current() const;

    bool commitWithIpkg();

    QString ipkgStatusOutput();
    QString ipkgInfoOutput();
    void setCachedIpkgOutputDirty();

    QString qcopDocument;
    bool committed;

    QCheckListItem* editedserver;
    int ipkg_old;
    void writeSettings();
    void readSettings();

    QDict<QString> *installedRootDict;
    
    QCString cachedIpkgStatusOutput;
    QCString cachedIpkgInfoOutput;
};

#endif // PACKAGEWIZARD_H
