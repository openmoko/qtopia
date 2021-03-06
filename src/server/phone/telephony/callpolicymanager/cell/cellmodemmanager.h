/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef _CELLMODEMMANAGER_H_
#define _CELLMODEMMANAGER_H_

#include <qcalllist.h>
#include <qspeeddial.h>
#include <qvaluespace.h>
#include <qsmsmessage.h>
#include <qcbsmessage.h>
#include <qnetworkregistration.h>
#include <qcallforwarding.h>
#include <qsupplementaryservices.h>
#include <QObject>
#include <QSound>
#include <QPinManager>
#include <QPhoneProfile>
#include <QPhoneProfileManager>
#include <QPhoneBook>
#include "qtopiaserverapplication.h"
#include "qabstractcallpolicymanager.h"

class QPinManager;
class DialerControl;
class CallHistory;
class CallScreen;
class QuickDial;
class Dialer;
class QSimCommand;
class MessageBox;
class ModemManager;

class QtopiaServiceDescription;
class QLabel;

class CellModemManagerPrivate;
class CellModemManager : public QAbstractCallPolicyManager
{
Q_OBJECT
public:
    CellModemManager(QObject *parent = 0);
    virtual ~CellModemManager();

    static void disableProfilesControlModem();
    static bool profilesControlModem();
    bool profilesBlocked();

    enum State { NoCellModem, Initializing, Initializing2,
                 Ready, WaitingSIMPin, VerifyingSIMPin,
                 WaitingSIMPuk, VerifyingSIMPuk,
                 SIMDead, SIMMissing, AerialOff,
                 FailureReset, UnrecoverableFailure };

    State state() const;

    QString callType() const;
    QString trCallType() const;
    QString callTypeIcon() const;
    QAbstractCallPolicyManager::CallHandling handling(const QString& number, QString& error);
    bool isAvailable(const QString& number);
    QString registrationMessage() const;
    QString registrationIcon() const;

    QString networkOperator() const;
    QString networkOperatorCountry() const;
    QTelephony::RegistrationState registrationState() const;
    bool callForwardingEnabled() const;
    bool simToolkitAvailable() const;
    bool cellModemAvailable() const;
    QString simToolkitIdleModeText() const;
    QIcon simToolkitIdleModeIcon() const;
    bool simToolkitIdleModeIconSelfExplanatory() const;
    QString simToolkitMenuTitle() const;

    bool planeModeEnabled() const;
    bool planeModeSupported() const;

    bool networkRegistered() const;

    static QString stateToString(State state);
    static QStringList emergencyNumbers();

    void setCellLocation( const QString &location );

public slots:
    void setPlaneModeEnabled(bool);
    void setSimPin(const QString &pin);
    void setSimPuk(const QString &puk, const QString &newPin);
    void blockProfiles(bool);

signals:
    void planeModeEnabledChanged(bool);
    void registrationStateChanged(QTelephony::RegistrationState);
    void networkOperatorChanged(const QString &);
    void stateChanged(CellModemManager::State newState,
                      CellModemManager::State oldState);
    void callForwardingEnabledChanged(bool);
    void simToolkitAvailableChanged(bool);

private slots:
    void rfLevelChanged();
    void pinStatus(const QString& type, QPinManager::Status status,
                   const QPinOptions&);
    void currentOperatorChanged();
    void registrationStateChanged();
    void autoRegisterTimeout();
    void planeModeChanged(bool);
    void queryCallForwarding();
    void forwardingStatus(QCallForwarding::Reason reason,
                          const QList<QCallForwarding::Status>& status);
    void setCallerIdRestriction();
    void simInserted();
    void simRemoved();
    void simToolkitAvailableChange();
    void simNotInserted();
    void readPhoneBooks();
    void fetchEmergencyNumbers();
    void fetchCallHistory();
    void emergencyNumbersFetched
        ( const QString& store, const QList<QPhoneBookEntry>& list );
    void callHistoryEntriesFetched
        ( const QString& store, const QList<QPhoneBookEntry>& list );
    void retryRfLevelRequest();

private:
    void doAutoRegister();
    void tryDoReady();
    void tryDoAerialOff();
    void newlyRegistered();
    void setNotReadyStatus();
    void setReadyStatus();
    void updateStatus();
    void setAerialEnabled(bool);
    void doStateChanged(State newState);
    void doInitialize();
    void doInitialize2();
    void startAutoRegisterTimer();
    void stopAutoRegisterTimer();
    CellModemManagerPrivate *d;
};
QTOPIA_TASK_INTERFACE(CellModemManager);

#endif // _CELLMODEMMANAGER_H_

