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


#ifndef PHONE_LAUNCHER
#define PHONE_LAUNCHER

#include <qmap.h>
#include <qdialog.h>
#include <qlist.h>
#include <qbasictimer.h>
#include <qwindowsystem_qws.h>
#include <QPointer>

#include <qsoftmenubar.h>
#include <qtopiaservices.h>
#include <quniqueid.h>
#include <qvaluespace.h>
#include "launcherview.h"
#include "qabstractmessagebox.h"
#include "homescreen.h"
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
class EarpieceVolume;
class QExportedBackground;
class QAbstractDialerScreen;
class TouchscreenDialer;
class QAbstractBrowserScreen;
class QAbstractSecondaryDisplay;
class QtopiaServiceDescription;

class QSpeedDialFeedback : public QFrame {
    Q_OBJECT
public:
    QSpeedDialFeedback();

    void show(QWidget* center, const QString& input, const QtopiaServiceDescription&);

protected:
    void keyReleaseEvent(QKeyEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void timerEvent(QTimerEvent*);

private:
    QtopiaServiceRequest req;
    QLabel *icon;
    QLabel *label;
    int timerId;
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
    CallHistory *callHistory() const { return mCallHistory; }
#endif

    QAbstractBrowserScreen* phoneBrowser(bool create = true) const;
    QAbstractSecondaryDisplay *secondaryDisplay(bool create = true) const;

    void hideAll();

public slots:
#ifdef QTOPIA_PHONEUI
    void showDialer(const QString &, bool speedDial = false);
    void showCallHistory(bool missed = false, const QString &hint = QString() );
    void showMissedCalls();
    void showCallScreen();
    void requestDial(const QString &n, const QUniqueId &c = QUniqueId());
    void dialNumber(const QString &n, const QUniqueId &c = QUniqueId(), const QString &callType = QString("Voice"));
    void acceptIncoming();
    void dialVoiceMail();
    void showSpeedDialer(const QString &);
#endif
    bool activateSpeedDial( const QString& input );
    void showProfileSelector();

signals:
    void windowRaised( QWSWindow *win );

protected slots:
    void polishWindows();
    void showEvent(QShowEvent *e);
    void sysMessage(const QString& message, const QByteArray&);
    void launchType(const QString& ltype);
    void showHomeScreen(int);
    void showPhoneLauncher();
#ifdef QTOPIA_PHONEUI
    void missedCount(int);
    void messageCountChanged(int, bool, bool);
    void messageRejected();
    void activeCallCount(int);
    void registrationChanged();
    void ussdMessage(const QString &);
#endif
    void callPressed();
    void hangupPressed();
    void multitaskPressed();
    void showRunningTasks();
    void keyStateChanged(bool);
    void updateBackground();
#ifdef QTOPIA_CELL
    void cellBroadcast(CellBroadcastControl::Type, const QString &, const QString &);
#endif
    void showContentSet();
protected:
    void initInfo();
    void resizeEvent(QResizeEvent *);
    void timerEvent(QTimerEvent *);

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
#endif
    void speedDial( const QString& input );
    void increaseEarVolume();
    void decreaseEarVolume();
    void rejectModalDialog();

private:
    friend class PhoneManager;
    void showWarning(const QString &title, const QString &text);

    int updateTid;

    PhoneHeader *header();
    PhoneHeader *m_header;

    void createContext();
    ContextLabel *context();
    ContextLabel *m_context;

    mutable QAbstractBrowserScreen *stack;
    HomeScreen *homeScreen;
    QMenu *documentsMenu;
    QPointer<QAbstractMessageBox> warningBox;
    int separatorId;
    int timeId;
    QBasicTimer multitaskingMultipressTimer;
    int multitaskingcursor;
    uint registrationMsgId;
#ifdef QTOPIA_PHONEUI
    int messageCount;
    int missedCallCount;
    int activeCalls;
#endif
    QStringList iconPath;
    bool slowUpdates;
    bool showAlerts;
#ifdef QTOPIA_PHONEUI
    QAbstractMessageBox *serviceMsgBox;
    QAbstractMessageBox *CBSMessageBox;
#endif
    EarpieceVolume *volumeScreen;

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
    int voipHideMsgTimer;
    bool voipHideMsg;
#endif
    QAbstractMessageBox *warningMsgBox;

#ifdef QTOPIA_PHONEUI
    int alertedMissed;
    int alertedMessageCount;
    bool messageBoxFull;
    bool isSysMsg;
    QString queuedCall;
    QString queuedCallType;
    QUniqueId queuedCallContact;
    bool queuedIncoming;

    bool waitingVoiceMailNumber;
#endif
    QSpeedDialFeedback *speeddialfeedback;
    QExportedBackground * m_exportedBackground;
    QString configuration;

    UIApplicationMonitor appMon;

#ifdef QTOPIA_PHONEUI
    QServiceNumbers *serviceNumbers;
#endif
#ifdef QTOPIA_CELL
    CellModemManager *cellModem;
#endif
};

#endif
