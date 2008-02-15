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

#ifndef DEVICE_UPDATER_H
#define DEVICE_UPDATER_H

#include "ui_deviceupdaterbase.h"

#include <QDialog>

class LocalSocketListener;
class PackageScanner;
class DeviceUpdaterBase_ui;
class Configure;
class ConfigureData;

class DeviceUpdaterWidget : public QWidget, public Ui::DeviceUpdaterBase
{
    Q_OBJECT
public:
    DeviceUpdaterWidget( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~DeviceUpdaterWidget();
    void connectLocalSocket( LocalSocketListener * ) const;
    void connectScanner( PackageScanner * ) const;
signals:
    void sendPackage(const QModelIndex & );
    void done(int);
public slots:
    void sendPackage(const QString & );
    void sendButtonClicked();
private slots:
    void progressWake();
    void quitWidget();
    void showConfigure();
    void scannerUpdated();
    void handleCommand( const QString & );
    void serverStopped();
    void serverStarted();
    void serverButtonToggled();
};

#endif
