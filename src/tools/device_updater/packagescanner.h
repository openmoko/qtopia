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

#ifndef DEVICEUPDATER_PACKAGESCANNER_H
#define DEVICEUPDATER_PACKAGESCANNER_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QStringList>
#include <QMutex>

#include "scannerthread.h"

// By default, this is the value for MAX on the progress bar
// so base all progress values from this
#define DEFAULT_PROGRESS_MAX 100

class DeviceConnector;

struct PackageItem
{
    QString qpkPath;
    QString qpdPath;
    QString name;
    QString display;
    QDateTime lastMod;
    bool hasBeenUploaded;
};


class PackageScanner : public QAbstractListModel
{
    Q_OBJECT
public:
    PackageScanner( const QString &, QObject *parent = 0 );
    ~PackageScanner();
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    Qt::ItemFlags flags(const QModelIndex&) const;
    PackageItem *findPackageByName( const QString & );
    void appendPackage( PackageItem *package );
public slots:
    void refresh();
    void sendPackage( const QModelIndex & );
    void sendPackage( const QString & );
signals:
    void progressMessage( const QString & );
    void progressValue( int );
    void updated();
private slots:
    void scannerDone();
    void connectorComplete();
private:
    void sendIt( PackageItem *pkg );
    QString mDir;
    QList<PackageItem*> mPackageList;
    mutable QMutex mPackageListMutex;
    ScannerThread *mScanner;
    DeviceConnector *mConnector;
    friend class ScannerThread;
};

#endif
