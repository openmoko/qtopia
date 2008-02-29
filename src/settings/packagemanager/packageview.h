/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA All rights reserved.
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

#ifndef PACKAGEVIEW_H
#define PACKAGEVIEW_H

#include <QMainWindow>
#include <QModelIndex>
#include <QtopiaAbstractService>
#include <QDSActionRequest>
#include <QTabWidget>

class PackageModel;
class QTreeView;
class QMenu;
class QActionGroup;
class QTextEdit;
class QAction;
class QShowEvent;
class QWaitWidget;

#include "ui_packagedetails.h"
class PackageDetails : public QDialog, public Ui::PackageDetails
{
public:
    enum Type { Info, Install, Uninstall, Reenable };
    PackageDetails(QWidget *parent, Type type = Info, bool modal = true);

    Q_OBJECT
};

class PackageView : public QMainWindow
{
    Q_OBJECT
    friend class PackageManagerService;
    friend class PackageServiceInstaller;
public:

    PackageView(QWidget* parent = 0, Qt::WFlags flags = 0);
    ~PackageView();

signals:
    void targetChoiceChanged( const QString & );
    void serverListUpdate( const QStringList &, const QStringList & );

private slots:
    void init();
    void editServers();
    void serversChanged( const QStringList & );
    void serverChoice( QAction* a );
    void targetsChanged( const QStringList & );
    void targetChoice( QAction* );
    void showDetails( const QModelIndex &, PackageDetails::Type type );
    void displayDetails();
    void startInstall();
    void startUninstall();
    void confirmReenable();
    void activateItem( const QModelIndex & );
    void contextMenuShow();
    void postServerStatus( const QString & );
    void selectNewlyInstalled( const QModelIndex & );
private:
    QTreeView *installedView;
    QTreeView *downloadView;
    QTabWidget *tabWidget;
    PackageModel *model;
    QString prevTarget;
    QLabel *statusLabel;
    QWaitWidget *waitWidget;

    QMenu *menuServers;
    QMenu *menuTarget;
    QActionGroup *targetActionGroup;
    QActionGroup *serversActionGroup;
    QAction *reenableAction;
    QAction *detailsAction;
    QAction *installAction;
    QAction *uninstallAction;

    static const int InstalledIndex;
    static const int DownloadIndex;
};

#endif
