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
class PhoneHeader;
class PhoneBrowser;
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
class QExportedBackground;
class QAbstractDialerScreen;
class QAbstractBrowserScreen;
class QAbstractSecondaryDisplay;
class QtopiaServiceDescription;
class GsmKeyActions;

class QSpeedDialFeedback : public QFrame {
    Q_OBJECT
public:
    QSpeedDialFeedback();

    void setBlindFeedback(bool on);
    void show(QWidget* center, const QString& input, const QtopiaServiceDescription&);

signals:
    void requestSent();

protected:
    void keyReleaseEvent(QKeyEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void timerEvent(QTimerEvent*);

private:
    QtopiaServiceRequest req;
    QLabel *icon;
    QLabel *label;
    int timerId;
    bool blind;
};

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
    QAbstractHomeScreen *homeScreen() const;
    CallHistory *callHistory() const { return mCallHistory; }
#endif

    QAbstractBrowserScreen* phoneBrowser(bool create = true) const;
    QAbstractSecondaryDisplay *secondaryDisplay(bool create = true) const;

    void hideAll();

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
    void showSpeedDialer(const QString &);
#endif
    bool activateSpeedDial( const QString& input );
    void showProfileSelector();
    void showWarning(const QString &title, const QString &text);

signals:
#ifdef Q_WS_QWS
    void windowRaised( QWSWindow *win );
#endif

protected slots:
    void polishWindows();
    void showEvent(QShowEvent *e);
    void sysMessage(const QString& message, const QByteArray&);
    void launchType(const QString& ltype);
    void showHomeScreen(int);
    void showPhoneLauncher();
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
    void callPressed();
    void hangupPressed();
    void multitaskPressed();
    void showRunningTasks();
    void keyStateChanged(bool);
    void updateBackground();
    void applySecondaryBackgroundImage();
#ifdef QTOPIA_CELL
    void cellBroadcast(CellBroadcastControl::Type, const QString &, const QString &);
#endif
    void showContentSet();
protected:
    void initInfo();
    void resizeEvent(QResizeEvent *);
    void timerEvent(QTimerEvent *);
    void closeEvent(QCloseEvent *);
    void paintEvent(QPaintEvent *);
    void updateLauncherIconSize();

private:
    void showMessageBox(const QString& title, const QString& text, QAbstractMessageBox::Icon=QAbstractMessageBox::Information);

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
    void speedDialActivated();
#endif
    void speedDial( const QString& input );
    void rejectModalDialog();
    void newMessagesChanged();

private:
    friend class PhoneManager;

    int updateTid;

    PhoneHeader *header();
    PhoneHeader *m_header;

    void createContext();
    ContextLabel *context();
    ContextLabel *m_context;

    mutable QAbstractBrowserScreen *stack;
    //HomeScreen *homeScreen;
    mutable QAbstractHomeScreen *m_homeScreen;
    QMenu *documentsMenu;
    QPointer<QAbstractMessageBox> warningBox;
    int separatorId;
    QBasicTimer multitaskingMultipressTimer;
    int multitaskingcursor;
    uint registrationMsgId;
#ifdef QTOPIA_PHONEUI
    int messageCount;
    int activeCalls;
    QValueSpaceItem newMessages;
#endif
    QStringList iconPath;
    bool slowUpdates;
#ifdef QTOPIA_PHONEUI
    QAbstractMessageBox *serviceMsgBox;
    QAbstractMessageBox *CBSMessageBox;
#endif

#ifdef QTOPIA_PHONEUI
    mutable CallScreen *mCallScreen;
    mutable QAbstractDialerScreen *m_dialer;
#endif
    mutable QAbstractSecondaryDisplay *secondDisplay;

#ifdef QTOPIA_PHONEUI
    CallHistory *mCallHistory;

    QAbstractMessageBox *missedMsgBox;
    QAbstractMessageBox *messagesMsgBox;
    QAbstractMessageBox *incomingMsgBox;
    QAbstractMessageBox *dialingMsgBox;
    QAbstractMessageBox *callTypeMsgBox;
#endif
#ifdef QTOPIA_VOIP
    QAbstractMessageBox *voipNoPresenceMsgBox;
#endif
    QAbstractMessageBox *warningMsgBox;

#ifdef QTOPIA_PHONEUI
    int alertedMissed;
    bool messageBoxFull;
    bool isSysMsg;
    QString queuedCall;
    QString queuedCallType;
    QUniqueId queuedCallContact;
    bool queuedIncoming;

    bool waitingVoiceMailNumber;
#endif
    QSpeedDialFeedback *speeddialfeedback;
    QString configuration;

    UIApplicationMonitor appMon;

#ifdef QTOPIA_PHONEUI
    QServiceNumbers *serviceNumbers;
#endif
#ifdef QTOPIA_CELL
    CellModemManager *cellModem;
    GsmKeyActions *gsmKeyActions;
#endif
#ifdef QTOPIA_PHONEUI
    bool dialerSpeedDialFeedbackActive;
#endif
};

#endif
