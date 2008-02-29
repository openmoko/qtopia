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

#ifndef PACKAGECONTROLLER_H
#define PACKAGECONTROLLER_H

#include <QString>
#include <QIcon>
#include <QList>
#include <QModelIndex>
#include <QSize>

#include "installcontrol.h"

class QProgressDialog;
class HttpFetcher;
class InstalledPackageScanner;
class PackageModel;

/**
  \class AbstractPackageController
  \internal
  Abstract base class for package controllers
*/
class AbstractPackageController : public QObject
{
    Q_OBJECT
public:
    enum PCType {
        local,
        network,
        installed
    };

    static const QString INFORMATION_FILE;
    static const QString PACKAGE_SUMMARY_FILE;
    static AbstractPackageController *factory( PCType t, PackageModel *parent = 0 );

    int numberPackages() const;
    InstallControl::PackageInfo packageInfo( int pkgId ) const;
    QList<InstallControl::PackageInfo> packageList() const { return pkgList; }

    virtual QVariant data( int ) const;
    virtual QVariant data( int, int, int ) const;

    virtual QIcon dataIcon() const = 0;
    virtual QString controllerName() const = 0;
    virtual QString controllerDescription() const = 0;
    virtual QIcon controllerIcon() const = 0;
    virtual QString packageInstallInfo( int pkgId ) const = 0;
    virtual QString operationName() const;
    virtual void install( int pkgId ) = 0;
    virtual void setPackageFilter( const QList<InstallControl::PackageInfo> &filter );
public slots:
    virtual void addPackage( const InstallControl::PackageInfo & );
signals:
    void packageMessage( const QString & );
    void packageInstalled( const InstallControl::PackageInfo & );
    void rowsAboutToBeRemoved( const QModelIndex &, int, int );
    void rowsRemoved( const QModelIndex &, int, int );
    void updated();
protected:
    AbstractPackageController( QObject *parent = 0 );
    virtual ~AbstractPackageController();

    QList<InstallControl::PackageInfo> pkgList;
    QList<InstallControl::PackageInfo> filteredOutPkgList;
    QProgressDialog *progressDisplay;
    InstallControl *installControl;
};

class PackageInformationReader;

#define PKG_WIZ_HTTP_INFO_FILE "packages.list"

/**
  \class LocalPackageController
  \internal
  Implement package controller for case of local packages, already
  available via the device filesystem
*/
class LocalPackageController : public AbstractPackageController
{
    Q_OBJECT
public:
    virtual QIcon dataIcon() const;
    virtual QString controllerName() const;
    virtual QIcon controllerIcon() const;
    virtual QString controllerDescription() const;
    virtual QString packageInstallInfo( int pkgId ) const;
    virtual void install( int pkgId );
protected:
    LocalPackageController( QObject *parent = 0 );
    ~LocalPackageController();
private slots:
    void packageComplete();
private:
    static const QString LOCAL_PACKAGE_DIRECTORY;

    void initialiseLocalPackageInfo();

    PackageInformationReader *reader;
    friend class AbstractPackageController;
};

/**
  \class NetworkPackageController
  \internal
  Implement package controller for case of packages available via the
  http network and which must be downloaded before installation
*/
class NetworkPackageController : public AbstractPackageController
{
    Q_OBJECT
public:
    virtual QIcon dataIcon() const;
    virtual QString controllerName() const;
    virtual QIcon controllerIcon() const;
    virtual QString controllerDescription() const;
    virtual QString packageInstallInfo( int pkgId ) const;
    virtual void install(int packageI);
    QString& networkServer();
    void setNetworkServer( const QString & );

protected:
    NetworkPackageController( QObject *parent = 0 );
    ~NetworkPackageController();

private slots:
    void packageFetchComplete();
    void insertNetworkPackageItems();
    void listFetchComplete();

private:
    QString currentNetworkServer;
    HttpFetcher *hf;
    friend class AbstractPackageController;
};

class QFileSystem;

/**
  \class InstalledPackageController
  \internal
  Implement package controller for packages already installed
*/
class InstalledPackageController : public AbstractPackageController
{
    Q_OBJECT
public:
    virtual QIcon dataIcon() const;
    virtual QString controllerName() const;
    virtual QIcon controllerIcon() const;
    virtual QString controllerDescription() const;
    virtual QString packageInstallInfo( int pkgId ) const;
    virtual QString operationName() const;
    virtual void install(int packageI);
public slots:
    void reloadInstalledLocations( const QStringList & );
protected:
    InstalledPackageController( QObject *parent = 0 );
    ~InstalledPackageController();
private slots:
    void initialize();
private:
    friend class AbstractPackageController;
    InstalledPackageScanner *installedScanner;
};

#ifdef Q_WS_QWS
// QtopiaCore #ifdef's QProgressDialog out of Qt
#include <QDialog>

class QProgressBar;
class QTextBrowser;
class QPushButton;
class QVBoxLayout;

class QProgressDialog : public QDialog
{
    Q_OBJECT
public:
    QProgressDialog( QWidget *p = 0, Qt::WFlags f = 0 );
    ~QProgressDialog();
    void setLabelText( const QString & );
    void setMaximum( int );
    virtual QSize sizeHint() const;
signals:
    void canceled();
public slots:
    void setValue( int );
    void reset();
private slots:
    void init();
private:
    QProgressBar *bar;
    QTextBrowser *label;
    QPushButton *cancelButton;
    QVBoxLayout *vb;
};

#endif

////////////////////////////////////////////////////////////////////////
/////
/////  AbstractPackageController inline method implementations
/////

inline int AbstractPackageController::numberPackages() const
{
    return pkgList.count();
}

inline InstallControl::PackageInfo AbstractPackageController::packageInfo( int pkgId ) const
{
    return pkgList[ pkgId ];
}

inline QVariant AbstractPackageController::data( int role ) const
{
    if ( role == Qt::DisplayRole )
        return controllerName();
    if ( role == Qt::DecorationRole )
        return controllerIcon();
    if ( role == Qt::UserRole )
        return controllerDescription();
    return QVariant();
}

inline QVariant AbstractPackageController::data( int row, int column, int role ) const
{
    InstallControl::PackageInfo pi = packageInfo( row );
    if ( role == Qt::DisplayRole )
        return ( column == 0 ) ? pi.name : pi.description;
    if ( role == Qt::DecorationRole )
        return dataIcon();
    if ( role == Qt::UserRole )
        return packageInstallInfo(row);
    return QVariant();
}

inline void AbstractPackageController::addPackage( const InstallControl::PackageInfo &pkg )
{
    pkgList.append( pkg );
    emit updated();
}

inline QString AbstractPackageController::operationName() const
{
    return tr( "Install" );
}

////////////////////////////////////////////////////////////////////////
/////
/////  LocalPackageController inline method implementations
/////

inline QIcon LocalPackageController::dataIcon() const
{
    return QIcon( ":icon/uninstalled" );
}

inline QString LocalPackageController::controllerName() const
{
    return tr( "Available locally" );
}

inline QIcon LocalPackageController::controllerIcon() const
{
    return QIcon( ":icon/folder" );
}

inline QString LocalPackageController::controllerDescription() const
{
    return tr( "Packages already downloaded available for installation" );
}

inline QString LocalPackageController::packageInstallInfo( int pkgId ) const
{
    return pkgList[ pkgId ].domain;
}

////////////////////////////////////////////////////////////////////////
/////
/////  NetworkPackageController inline method implementations
/////

inline QIcon NetworkPackageController::dataIcon() const
{
    return QIcon( ":icon/uninstalled" );
}

inline QString NetworkPackageController::controllerName() const
{
    return tr( "Downloads available" );
}

inline QIcon NetworkPackageController::controllerIcon() const
{
    return QIcon( ":icon/irreceive" );
}

inline QString& NetworkPackageController::networkServer()
{
    return currentNetworkServer;
}

inline QString NetworkPackageController::controllerDescription() const
{
    return tr( "Network packages for download from servers:<br/>%1" )
        .arg( currentNetworkServer );
}

/*!
  \internal
  Return the raw domain string here, and let the view do post-processing to
  add the human-readable description
*/
inline QString NetworkPackageController::packageInstallInfo( int pkgId ) const
{
    if ( pkgId < pkgList.count() )
        return pkgList[ pkgId ].domain;
    else
        return QString();
}

////////////////////////////////////////////////////////////////////////
/////
/////  InstalledPackageController inline method implementations
/////

inline QIcon InstalledPackageController::dataIcon() const
{
    return QIcon( ":icon/installed" );
}

inline QString InstalledPackageController::controllerName() const
{
    return tr( "Installed" );
}

inline QIcon InstalledPackageController::controllerIcon() const
{
    return QIcon( ":icon/install" );
}

inline QString InstalledPackageController::controllerDescription() const
{
    return tr( "Currently installed packages which may be removed to regain space" );
}

inline QString InstalledPackageController::packageInstallInfo( int pkgId ) const
{
    return tr( "Installed package <font color=\"#0000FF\">%1</font> %2",
            "%1=name of pkg, %2=short description" ).arg( pkgList[pkgId].name )
        .arg( pkgList[pkgId].description );
}

inline QString InstalledPackageController::operationName() const
{
    return tr( "Uninstall" );
}

#endif
