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


#ifndef PHONE_LAUNCHER
#define PHONE_LAUNCHER

#include <qmap.h>
#include <qdialog.h>
#include <qlist.h>
#include <qbasictimer.h>
#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif
#include <QPointer>

#include <qsoftmenubar.h>
#include <qtopiaservices.h>
#include <quniqueid.h>
#include <qvaluespace.h>
#include "launcherview.h"
#include "qabstractmessagebox.h"
#include "applicationmonitor.h"

#ifdef QTOPIA_PHONEUI
#include <qservicenumbers.h>
#endif

#ifdef QTOPIA_CELL
#include "cellmodemmanager.h"
#include "cellbroadcastcontrol.h"
#endif
#include "qabstractserverinterface.h"

class HomeScreen;
class QAbstractHomeScreen;
class ContextLabel;
class PhoneMainMenu;
class LazyContentStack;
class QCategoryDialog;
class QAbstractMessageBox;
class TypeDialog;
class InputMethods;
class QLabel;
class QSettings;
class QAction;
class CallScreen;
class QuickDial;
class CallHistory;
class QAbstractDialerScreen;
class QAbstractBrowserScreen;
class QtopiaServiceDescription;
class GsmKeyActions;

class PhoneLauncher : public QAbstractServerInterface
{
    Q_OBJECT
public:
    PhoneLauncher(QWidget *parent = 0, Qt::WFlags fl = Qt::FramelessWindowHint);
    ~PhoneLauncher();

#ifdef QTOPIA_PHONEUI
    void showAlertDialogs();

    CallScreen *callScreen(bool create = true) const;
    QAbstractDialerScreen *dialer(bool create = true) const;
    CallHistory *callHistory() const { return m_callHistory; }
#endif

public slots:
#ifdef QTOPIA_PHONEUI
    void showDialer(const QString &, bool speedDial = false);
    void showCallHistory(bool missed = false, const QString &hint = QString());
    void showMissedCalls();
    void showCallScreen();
    void requestDial(const QString &n, const QUniqueId &c = QUniqueId());
    void dialNumber(const QString &n, 
		    const QUniqueId &c = QUniqueId(), 
		    const QString &callType = QString("Voice"));
    void acceptIncoming();
    void dialVoiceMail();
#endif
    void showWarning(const QString &title, const QString &text);

protected slots:
    void polishWindows();
    void showEvent(QShowEvent *e);
    void sysMessage(const QString& message, const QByteArray&);
#ifdef QTOPIA_PHONEUI
    void missedCount(int);
    void messageCountChanged(int, bool, bool, bool);
    void smsMemoryFull(bool);
    void messageRejected();
    void activeCallCount(int);
    void registrationChanged();
    void ussdMessage(const QString &);
    void initializeCallHistory();
#endif
#ifdef QTOPIA_CELL
    void cellBroadcast(CellBroadcastControl::Type, const QString &, const QString &);
#endif
protected:
    void initInfo();
    void resizeEvent(QResizeEvent *);
    void closeEvent(QCloseEvent *);
    void paintEvent(QPaintEvent *);
    void updateLauncherIconSize();

private slots:
#ifdef QTOPIA_CELL
    void unstructuredNotification (QSupplementaryServices::UnstructuredAction action, const QString& data);
#endif
    void loadTheme();
#ifdef QTOPIA_PHONEUI
    void serviceNumber(QServiceNumbers::NumberId id, const QString& number);
    void messageBoxDone(int);
    void stateChanged();
    void resetMissedCalls();
#endif
    void rejectModalDialog();
    void newMessagesChanged();

private:
    int m_updateTid;

    void createContext();
    ContextLabel *context();
    ContextLabel *m_context;

    //HomeScreen *homeScreen;
    mutable QAbstractHomeScreen *m_homeScreen;
    uint m_registrationMessageId;

    QAbstractMessageBox *m_warningMessageBox;
    UIApplicationMonitor m_appMon;

#ifdef QTOPIA_PHONEUI
    int m_messageCount;
    int m_activeCalls;
    QValueSpaceItem m_newMessages;
    QAbstractMessageBox *m_serviceMessageBox;
    QAbstractMessageBox *m_CBSMessageBox;
    mutable CallScreen *m_callScreen;
    mutable QAbstractDialerScreen *m_dialer;
    CallHistory *m_callHistory;

    QAbstractMessageBox *m_missedMessageBox;
    QAbstractMessageBox *m_messagesMessageBox;
    QAbstractMessageBox *m_incomingMessageBox;
    QAbstractMessageBox *m_dialingMessageBox;
    QAbstractMessageBox *m_callTypeMessageBox;
    int m_alertedMissed;
    bool m_messageBoxFull;
    bool m_isSystemMessage;
    QString m_queuedCall;
    QString m_queuedCallType;
    QUniqueId m_queuedCallContact;
    bool m_queuedIncoming;

    bool m_waitingVoiceMailNumber;
    QServiceNumbers *m_serviceNumbers;
#endif
#ifdef QTOPIA_VOIP
    QAbstractMessageBox *m_voipNoPresenceMessageBox;
#endif

#ifdef QTOPIA_CELL
    CellModemManager *m_cellModem;
    GsmKeyActions *m_gsmKeyActions;
#endif
};

#endif
