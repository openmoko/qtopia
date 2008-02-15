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

#ifndef IAXSETTINGS_H
#define IAXSETTINGS_H

#include <qdialog.h>
#include "ui_iaxsettingsbase.h"
#include <qnetworkregistration.h>
#include <qtelephonyconfiguration.h>

class QAction;

class IaxSettings : public QDialog
{
    Q_OBJECT
public:
    IaxSettings( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~IaxSettings();

protected:
    void accept();
    void reject();

private slots:
    void actionRegister();
    void updateRegister();
    void registrationStateChanged();

private:
    Ui::IaxSettingsBase *settings;
    QAction *registerAction;
    bool registered;
    QNetworkRegistration *netReg;
    QTelephonyConfiguration *config;

    QString savedUserId;
    QString savedPassword;
    QString savedHost;
    bool savedAutoRegister;
    QString savedCallerIdNumber;
    QString savedCallerIdName;

    void copyToWidgets();
    void copyFromWidgets();
    bool isRegistrationChanged() const;
    bool isCallerIdChanged() const;

    void updateRegistrationConfig();
    void updateCallerIdConfig();
    void registerToProxy();
    void deregisterFromProxy();
};

#endif // IAXSETTINGS_H
