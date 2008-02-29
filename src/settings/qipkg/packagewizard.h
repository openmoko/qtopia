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
