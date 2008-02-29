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

#ifndef SIPSETTINGS_H
#define SIPSETTINGS_H

#include <qdialog.h>
#include "ui_sipsettingsbase.h"
#include <qnetworkregistration.h>
#include <qtelephonyconfiguration.h>

class QAction;
class QPresence;

class SipSettings : public QDialog
{
    Q_OBJECT
public:
    SipSettings( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~SipSettings();

protected:
    void accept();
    void reject();

private slots:
    void actionRegister();
    void actionAvailable();
    void updateRegister();
    void updateAvailable();
    void registrationStateChanged();
    void presenceChanged();
    void appMessage( const QString &, const QByteArray & );

private:
    Ui::SipSettingsBase *settings;
    QAction *registerAction;
    QAction *availableAction;
    bool registered;
    bool available;
    QPresence *presence;
    QNetworkRegistration *netReg;
    QTelephonyConfiguration *config;

    int savedExpires;
    int savedSubscribeExpires;
    QString savedUserUri;
    QString savedProxy;
    QString savedProxyUserName;
    QString savedProxyPassword;
    bool savedAutoRegister;
    QString savedCodec;

    void copyToWidgets();
    void copyFromWidgets();
    bool isChanged() const;

    void updateRegistrationConfig();
    void registerToProxy();
    void deregisterFromProxy();
};

#endif // SIPSETTINGS_H
