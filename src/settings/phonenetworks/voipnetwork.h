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

#ifndef VOIPNETWORKREGISTER_H
#define VOIPNETWORKREGISTER_H

#include <qnetworkregistration.h>
#include <qpresence.h>
#include <qtelephonyconfiguration.h>

#include <QListWidget>

class QModemNetworkRegistration;
class QWaitWidget;

class VoipNetworkRegister : public QListWidget
{
    Q_OBJECT
public:
    VoipNetworkRegister( QWidget *parent = 0 );
    ~VoipNetworkRegister();

protected:
    void showEvent( QShowEvent * );

private slots:
    void operationSelected( QListWidgetItem * );
    void registrationStateChanged();
    void localPresenceChanged();

private:
    QNetworkRegistration *m_client;
    QPresence *m_presence;
    QTelephonyConfiguration *m_config;
    QListWidgetItem *m_regItem, *m_presenceItem, *m_configItem;

    void init();
    inline bool registered() { return m_client->registrationState() == QTelephony::RegistrationHome; }
    inline bool visible() { return m_presence->localPresence() == QPresence::Available; }
    void updateRegistrationState();
    void updatePresenceState();
    void configureVoIP();
};

#endif /* VOIPNETWORKREGISTER_H */
