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

#include <qpowerstatus.h>
#include <qtopiaapplication.h>

#include "qtopiaserverapplication.h"
#include <qtopiaipcenvelope.h>
#include <qtopiaipcadaptor.h>
#include <qmimetype.h>
#include <qtranslatablesettings.h>
#include <qtopiaservices.h>
#include <qdocumentproperties.h>
#include <qcategoryselector.h>
#include <qsoftmenubar.h>
#include <qtopianamespace.h>
#include <qtopiaservices.h>
#include <qspeeddial.h>
#include <qtopialog.h>
#include <qtopianamespace.h>
#include <custom.h>
#include <themedview.h>
#include <qtopiabase/private/qsharedmemorycache_p.h>
#include <qdrmcontent.h>
#include <qexportedbackground.h>
#include <qwaitwidget.h>
#include <QPhoneProfile>
#include <QPhoneProfileManager>
#include <qpainter.h>
#include <qdir.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qaction.h>
#include <qevent.h>
#include <qpixmapcache.h>
#ifndef MEDIAPLAYER_RINGTONES
#include <qsoundqss_qws.h>
#endif
#include <qscreen_qws.h>
#include <qvariant.h>
#include <QStackedWidget>
#include <QTimer>
#include <QDesktopWidget>
#include <QDebug>
#include <QMenu>
#include <QScrollArea>

#include "phonelauncher.h"
#include "phone/ui/phonelauncherview.h"
#include "windowmanagement.h"
#include "inputmethods.h"
#include "contextlabel.h"
#include "messagebox.h"

#include "contentserver.h"
#include "phonebrowser.h"
#include "homescreen.h"
#include "documentview.h"
#include "phoneheader.h"
#include "alarmcontrol.h"
#include "taskmanagerservice.h"
#include "homescreen.h"
#include "themecontrol.h"
#include "qabstractsecondarydisplay.h"
#include "contentsetlauncherview.h"

#ifdef QTOPIA_VOIP
#include "voipmanager.h"
#endif

#ifdef QTOPIA_PHONEUI
#include "ringcontrol.h"
#include "dialer.h"
#include "quickdial.h"
#include "dialercontrol.h"
#include "dialerservice.h"
#include "callscreen.h"
#include "callhistory.h"
#include "dialercontrol.h"
#include "messagecontrol.h"
#endif

#define QTOPIA_ENABLE_EXPORTED_BACKGROUNDS      // experimental

// declare EarpieceVolume
class EarpieceVolume : public QWidget
{
    Q_OBJECT
public:
    EarpieceVolume(QWidget *parent=0, Qt::WFlags f=0);

    void increaseVolume();
    void decreaseVolume();

protected slots:
    void levelChanged(int v);

protected:
    void timerEvent(QTimerEvent *e);
    bool eventFilter(QObject *o, QEvent *e);

private:
    int tid;
    int minLevel;
    int maxLevel;
    int level;
    QLabel *label;
    QSlider *slider;
};

class RejectDlg : public QDialog
{
public:
    void rejectDlg() { reject(); }
    void hideDlg() { hide(); }
};


extern QWSServer *qwsServer;

// declare MultiTaskProxy
class MultiTaskProxy : public TaskManagerService
{
Q_OBJECT
public:
    MultiTaskProxy(QObject * = 0);

protected:
    virtual void multitask();
    virtual void showRunningTasks();

signals:
    void doMultiTask();
    void doShowRunningTasks();
};

#ifdef QTOPIA_PHONEUI
// declare DialerServiceProxy
class DialerServiceProxy : public DialerService
{
Q_OBJECT
public:
    DialerServiceProxy(QObject * = 0);

protected:
    virtual void dialVoiceMail();
    virtual void dial( const QString& name, const QString& number );
    virtual void dial( const QString& number, const QUniqueId& contact );
    virtual void showDialer( const QString& digits );

signals:
    void doDialVoiceMail();
    void doDial( const QString& number );
    void doDial( const QString& number, const QUniqueId& contact );
    void doShowDialer( const QString& digits );
};

// define DialerServiceProxy
DialerServiceProxy::DialerServiceProxy(QObject *parent)
: DialerService(parent)
{
}

void DialerServiceProxy::dialVoiceMail()
{
    emit doDialVoiceMail();
}

void DialerServiceProxy::dial( const QString&, const QString& number )
{
    emit doDial(number);
}

void DialerServiceProxy::dial( const QString& number, const QUniqueId& contact )
{
    emit doDial(number, contact);
}

void DialerServiceProxy::showDialer( const QString& digits )
{
    if( !DialerControl::instance()->hasIncomingCall() &&
        !DialerControl::instance()->isDialing() ) {
        emit doShowDialer(digits);
    }
}
#endif

// define PhoneLauncher
PhoneLauncher::PhoneLauncher(QWidget *parent, Qt::WFlags fl)
    : QAbstractServerInterface(parent, fl), updateTid(0), m_header(0),
      m_context(0), stack(0), documentsMenu(0), timeId(0),
      registrationMsgId(0),
#ifdef QTOPIA_PHONEUI
      messageCount(0), missedCallCount(0), activeCalls(0),
#endif
      slowUpdates(false), showAlerts(false),
#ifdef QTOPIA_PHONEUI
      serviceMsgBox(0), CBSMessageBox(0),
#endif
      volumeScreen(0),
#ifdef QTOPIA_PHONEUI
    mCallScreen(0), m_dialer(0),
#endif
    secondDisplay(0),
#ifdef QTOPIA_PHONEUI
    mCallHistory(0),
    missedMsgBox(0), messagesMsgBox(0), incomingMsgBox(0),
    dialingMsgBox(0), callTypeMsgBox(0),
#endif
#ifdef QTOPIA_VOIP
    voipNoPresenceMsgBox(0),
    voipHideMsgTimer(0), voipHideMsg(false),
#endif
    warningMsgBox(0),
#ifdef QTOPIA_PHONEUI
    alertedMissed(0),
    alertedMessageCount(0),
    messageBoxFull(false),
    isSysMsg(false),
    queuedIncoming(false),
    waitingVoiceMailNumber(false),
#endif
    speeddialfeedback(0),
    m_exportedBackground(0)
#ifdef QTOPIA_CELL
    , cellModem(0)
#endif
{
    configuration = Qtopia::defaultButtonsFile();

    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect = desktop->screenGeometry(desktop->primaryScreen());
    QExportedBackground::initExportedBackground(desktopRect.width(),
                                                desktopRect.height(),
                                                desktop->primaryScreen());
    m_exportedBackground = new QExportedBackground(desktop->primaryScreen(), this);

    // Create callscreen
//    callScreen();

#ifdef QTOPIA_CELL
    CellBroadcastControl *cbc = CellBroadcastControl::instance();
    connect(cbc,
            SIGNAL(broadcast(CellBroadcastControl::Type,QString,QString)),
            this,
            SLOT(cellBroadcast(CellBroadcastControl::Type,QString,QString)));

    QSupplementaryServices *supp = new QSupplementaryServices( "modem", this );
    connect( supp, SIGNAL(unstructuredNotification(QSupplementaryServices::UnstructuredAction,QString)),
             this, SLOT(unstructuredNotification(QSupplementaryServices::UnstructuredAction,QString)) );
#endif

#ifndef QT_NO_TRANSLATION //load translation for profile names
    QtopiaApplication::loadTranslations("QtopiaDefaults");
#endif
    setGeometry(desktopRect);

    QObject::connect(ThemeControl::instance(), SIGNAL(themeChanged()),
                     this, SLOT(loadTheme()));

    // Create phone header
    header();

    // Create phone context bar
    createContext();

    // Create home screen
    homeScreen = HomeScreen::getInstancePtr();
    homeScreen->setParent(this); // Created as singleton - reparent here
    homeScreen->setGeometry(desktopRect);
    QObject::connect(homeScreen, SIGNAL(speedDial(const QString &)),
                     this, SLOT(activateSpeedDial(const QString &)));
    ThemeControl::instance()->registerThemedView(homeScreen, "Home");

#ifdef QTOPIA_PHONEUI
    QObject::connect(homeScreen, SIGNAL(showCallScreen()),
                     this, SLOT(showCallScreen()));
    QObject::connect(homeScreen, SIGNAL(showMissedCalls()),
                     this, SLOT(showMissedCalls()));
    QObject::connect(homeScreen, SIGNAL(showCallHistory()),
                     this, SLOT(showCallHistory()));
    QObject::connect(homeScreen, SIGNAL(dialNumber(const QString &)),
                     this, SLOT(showSpeedDialer(const QString &)));
    QObject::connect(homeScreen, SIGNAL(callEmergency(QString)),
                     this, SLOT(requestDial(QString)));
#endif

    QObject::connect(homeScreen, SIGNAL(keyLockedChanged(bool)),
                     this, SLOT(keyStateChanged(bool)));
    QObject::connect(homeScreen, SIGNAL(showPhoneBrowser()),
                     this, SLOT(showPhoneLauncher()));

    // Create secondary display
    secondaryDisplay();

    // Implement multi task service
    MultiTaskProxy *multiTaskProxy = new MultiTaskProxy(this);
    connect(multiTaskProxy, SIGNAL(doMultiTask()),
            this, SLOT(multitaskPressed()));
    connect(multiTaskProxy, SIGNAL(doShowRunningTasks()),
            this, SLOT(showRunningTasks()));

#ifdef QTOPIA_PHONEUI
    // Implement dialer service
    DialerServiceProxy *dialerServiceProxy = new DialerServiceProxy(this);
    connect(dialerServiceProxy, SIGNAL(doDialVoiceMail()),
            this, SLOT(dialVoiceMail()));
    connect(dialerServiceProxy, SIGNAL(doDial(const QString &)),
            this, SLOT(requestDial(const QString &)));
    connect(dialerServiceProxy, SIGNAL(doDial(const QString &, const QUniqueId &)),
            this, SLOT(requestDial(const QString &, const QUniqueId &)));
    connect(dialerServiceProxy, SIGNAL(doShowDialer(const QString &)),
            this, SLOT(showDialer(const QString &)));
#endif

    // Listen to system channel
    QtopiaChannel* sysChannel = new QtopiaChannel( "QPE/System", this );
    connect( sysChannel, SIGNAL(received(const QString&,const QByteArray&)),
             this, SLOT(sysMessage(const QString&,const QByteArray&)) );

    phoneBrowser()->resetToView("Main"); // better to get initial icon load cost now, rather then when user clicks.

    showHomeScreen(0);
    homeScreen->setFocus();

#ifdef QTOPIA_PHONEUI
    connect(DialerControl::instance(), SIGNAL(missedCount(int)),
            this, SLOT(missedCount(int)));
    connect(MessageControl::instance(), SIGNAL(messageCount(int,bool,bool)),
            this, SLOT(messageCountChanged(int,bool,bool)));
    connect(MessageControl::instance(), SIGNAL(messageRejected()),
            this, SLOT(messageRejected()));
    connect(DialerControl::instance(), SIGNAL(activeCount(int)),
            this, SLOT(activeCallCount(int)));
    connect(DialerControl::instance(), SIGNAL(autoAnswerCall()),
            this, SLOT(acceptIncoming()));
    connect(DialerControl::instance(), SIGNAL(stateChanged()),
            this, SLOT(stateChanged()));
#endif

#ifdef QTOPIA_CELL
    cellModem = qtopiaTask<CellModemManager>();
    connect(cellModem,
            SIGNAL(registrationStateChanged(QTelephony::RegistrationState)),
            this,
            SLOT(registrationChanged()));
    connect(cellModem, SIGNAL(planeModeEnabledChanged(bool)),
            this, SLOT(registrationChanged()));
#endif
#ifdef QTOPIA_VOIP
    connect(VoIPManager::instance(),
            SIGNAL(registrationChanged(QTelephony::RegistrationState)),
            this,
            SLOT(registrationChanged()));
#endif


#ifdef QTOPIA_PHONEUI
    // Voice mail
    serviceNumbers = new QServiceNumbers( QString(), this );
    connect( serviceNumbers, SIGNAL(serviceNumber(QServiceNumbers::NumberId,QString)),
             this, SLOT(serviceNumber(QServiceNumbers::NumberId,QString)) );

    messageCount = MessageControl::instance()->messageCount();

    // Don't alert user until count changes.
    alertedMissed = DialerControl::instance()->missedCallCount();
    alertedMessageCount = messageCount;
#endif
//    ThemeControl::instance()->refresh();

    connect( ContentSetViewService::instance(), SIGNAL(showContentSet()),
             this, SLOT(showContentSet()) );

#ifdef QTOPIA_PHONEUI
    registrationChanged();
#endif

    loadTheme();
}

PhoneLauncher::~PhoneLauncher()
{
    delete stack;
    delete m_header;
    delete m_context;
#ifdef QTOPIA_PHONEUI
    delete mCallScreen;
    if(mCallHistory)
        delete mCallHistory;
    if( m_dialer )
        delete m_dialer;
    delete speeddialfeedback;
#endif
    if ( secondDisplay )
        delete secondDisplay;
}

void PhoneLauncher::showEvent(QShowEvent *e)
{
    QTimer::singleShot(0, homeScreen, SLOT(show()));
#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
    QTimer::singleShot(0, this, SLOT(updateBackground()));
#endif
    header()->show();
    if (QSoftMenuBar::keys().count())
        context()->show();
    QWidget::showEvent(e);
}

void PhoneLauncher::callPressed()
{
#ifdef QTOPIA_PHONEUI
    // Called if server windows are not on top
    showCallHistory();
#endif
    if (warningBox)
        delete warningBox;
}

void PhoneLauncher::hangupPressed()
{
    // Called if server windows are not on top
    topLevelWidget()->raise();
    QtopiaIpcEnvelope e("QPE/System","close()");
    hideAll();
    showHomeScreen(0);
}

void PhoneLauncher::multitaskPressed()
{
    // When pressed, home screen is shown (without quiting apps).
    // When pressed again soon after (X msec), next running app is shown.

    QStringList runningApps = appMon.runningApplications();

    QStringList sortedapps = runningApps;
    sortedapps.removeAll("sipagent"); // workaround
    sortedapps.sort();

    int to_run=-1;
    if ( homeScreen->isActiveWindow() ) {
        // first application
        if ( sortedapps.count() == 0 ) {
            showRunningTasks(); // (gives error message)
            return;
        }
        to_run=multitaskingcursor=0;
    } else if ( multitaskingMultipressTimer.isActive() ) {
        // next application...
        if ( ++multitaskingcursor >= sortedapps.count() ) {
            to_run = -1;
        } else {
            to_run=multitaskingcursor;
        }
    }
    if ( to_run < 0 ) {
        multitaskingcursor=0;
        showHomeScreen(0);
    } else {
        QContentFilter filter(QContent::Application);
        QContentSet set(filter);
        QStringList sortedapps = runningApps;
        sortedapps.sort();
        QContent app = set.findExecutable(sortedapps[to_run]);
        multitaskingMultipressTimer.start(2500,this);
        if ( app.isValid() )
            app.execute();
    }
}

void PhoneLauncher::showRunningTasks()
{
    QStringList runningApps = appMon.runningApplications();

    //if ( runningApps.count() > 0 ) {
        hideAll();
        launchType("Folder/Running");
        phoneBrowser()->showMaximized();
        phoneBrowser()->raise();
        phoneBrowser()->activateWindow();
        phoneBrowser()->topLevelWidget()->setWindowTitle(tr("Running"));

    //} else {
        // The window title is "Multitasking", to hint to the user what
        // is happening. This makes the UI more discoverable.
        //showMessageBox(tr("Multitasking"), tr("No applications are running."));
    //}
}

void PhoneLauncher::showContentSet()
{
    hideAll();
    phoneBrowser()->moveToView("Folder/ContentSet");
    phoneBrowser()->showMaximized();
    phoneBrowser()->raise();
    phoneBrowser()->activateWindow();
}

void PhoneLauncher::loadTheme()
{
    bool v = isVisible();

    qLog(UI) << "Load theme";
    homeScreen->hide();

    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect = desktop->screenGeometry(desktop->primaryScreen());

    // header - not lazy
    WindowManagement::dockWindow(header(), WindowManagement::Top, header()->reservedSize());
    if ( v ) header()->show();

    // context bar - not lazy
    WindowManagement::dockWindow(context(), WindowManagement::Bottom, context()->reservedSize());
    if (v && QSoftMenuBar::keys().count())
        context()->show();

    // home screen - not lazy
    homeScreen->setGeometry(desktopRect);

#ifdef QTOPIA_PHONEUI
    // call screen - lazy
    CallScreen *cs = callScreen(false);
    if(cs) {
        cs->setGeometry(desktopRect.x(), desktopRect.y()+header()->sizeHint().height(),
                        desktopRect.width(),
                        desktopRect.height() - header()->sizeHint().height() - context()->height() );
    }
#endif

    // update position of launcher stack - lazy
    if(phoneBrowser(false)) {
        if ( !phoneBrowser()->isHidden() )
            phoneBrowser()->showMaximized();
        else
            phoneBrowser()->setGeometry(desktopRect.x(), desktopRect.y()+header()->sizeHint().height(),
                        desktopRect.width(),
                        desktopRect.height() - header()->sizeHint().height() - context()->height());
    }

    initInfo();

    if ( v ) {
        QTimer::singleShot(0, homeScreen, SLOT(show()));
#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
        QTimer::singleShot(0, this, SLOT(updateBackground()));
#endif
    }
}

#ifdef QTOPIA_PHONEUI
void PhoneLauncher::showMissedCalls()
{
    showCallHistory(true);
}

/*!
  Displays the call screen.
*/
void PhoneLauncher::showCallScreen()
{
    if( !callScreen()->sourceLoaded() ) {
        callScreen()->loadSource();
    }
    rejectModalDialog();
    callScreen()->showMaximized();
    callScreen()->raise();
}
#endif

void PhoneLauncher::initInfo()
{
    AlarmControl::instance();

    update();

    timerEvent(0);

#ifdef QTOPIA_PHONEUI
    int missCalls = DialerControl::instance()->missedCallCount();
    if ( missCalls != 0 )
        missedCount(missCalls);
#endif
}

void PhoneLauncher::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
}

void PhoneLauncher::sysMessage(const QString& message, const QByteArray &data)
{
    QDataStream stream( data );
    if ( message == "showHomeScreen()" ) {
        showHomeScreen(0);
    } else if ( message == QLatin1String("showHomeScreenAndToggleKeylock()") ) {
        showHomeScreen(2);
    } else if ( message == "applyStyleSplash()" ) {
        QWaitWidget *waitWidget = new QWaitWidget( this );
        waitWidget->show();
        qApp->processEvents();
        ThemeControl::instance()->refresh();
        polishWindows();
        updateLauncherIconSize();
        delete waitWidget;
    } else if ( message == "applyStyleNoSplash()" ) {
        qApp->processEvents();
        ThemeControl::instance()->refresh();
        polishWindows();
        updateLauncherIconSize();
        QtopiaChannel::send( "QPE/Application/appearance", "Settings::themeLoaded()" );
    } else if ( message == "applyBackgroundImage()" ) {
        homeScreen->applyBackgroundImage();
        updateBackground();
    } else if ( message == "serverKey(int,int)" ) {
        int key,press;
        stream >> key >> press;
        if ( key == Qt::Key_Call && press ) {
            callPressed();
        } else if ( (key == Qt::Key_Hangup || key == Qt::Key_Flip) && press ) {
            hangupPressed();
        }
    }
}

#include "../contentserver.h"

void PhoneLauncher::launchType(const QString& type)
{
    phoneBrowser()->resetToView(type);
}

void PhoneLauncher::showMessageBox(const QString& title, const QString& text, QAbstractMessageBox::Icon icon)
{
    if (!warningBox) {
        warningBox = QAbstractMessageBox::messageBox(this, title, text, icon);
        warningBox->setAttribute(Qt::WA_DeleteOnClose); // It's a QPointer<> so safe.
    }
    warningBox->setText(text);
    QtopiaApplication::showDialog(warningBox);
}

void PhoneLauncher::showHomeScreen(int state)
{
    // state: 0 -> no screensaver calls
    //        1 -> showHomeScreen called by screensaver
    //        2 -> showHomeScreen called when lock key is pressed

#ifdef QTOPIA_PHONEUI
    if (state != 0 && activeCalls) {
        return;
    }
#endif

    rejectModalDialog();
    topLevelWidget()->raise();
    homeScreen->raise();
    homeScreen->show();
    homeScreen->setFocus();
    topLevelWidget()->activateWindow();

    if(phoneBrowser(false)) {
        phoneBrowser()->hide();
    }

    if (timeId)
        killTimer(timeId);
    timeId = startTimer(60000); // slow down date and time updates
    slowUpdates = true;

    if (warningBox)
        delete warningBox;

    if (state == 1) {
        //QtopiaIpcEnvelope closeApps( "QPE/System", "close()" );
        if (!Qtopia::mousePreferred()) {
            QSettings c("Trolltech","qpe");
            c.beginGroup("HomeScreen");
            QString lockType = c.value( "AutoKeyLock", "Disabled" ).toString();
            if (lockType == "Enabled")
                homeScreen->setKeyLocked(true);
        }
    } else if (state == 2) {
        if (!homeScreen->keyLocked())
            homeScreen->setKeyLocked(true);
        else
            qwsServer->processKeyEvent(0, BasicKeyLock::lockKey(), 0, true, false);
    }
}

void PhoneLauncher::rejectModalDialog()
{
    // Last resort.  We shouldn't have modal dialogs in the server, but
    // just in case we need to get rid of them when a call arrives.  This
    // is a bad thing to do, but far less dangerous than missing a call.
    // XXX Known modals:
    //  - category edit dialog
    QWidgetList list = QApplication::topLevelWidgets();
    QList<QPointer<RejectDlg> > dlgsToDelete;

    foreach(QWidget *w, list)
        if (w->isVisible() && w->inherits("QDialog"))
            dlgsToDelete.append((RejectDlg*)w);

    foreach(QPointer<RejectDlg> d, dlgsToDelete) {
        if (!d)
            continue;

        if (d->testAttribute(Qt::WA_ShowModal)) {
            qWarning("Rejecting modal dialog: %s", d->metaObject()->className());
            d->rejectDlg();
        } else {
            qWarning("Hiding non-modal dialog: %s", d->metaObject()->className());
            d->hideDlg();
        }
    }
}

void PhoneLauncher::showPhoneLauncher()
{
    phoneBrowser()->resetToView("Main");

    phoneBrowser()->showMaximized();
    phoneBrowser()->raise();

    slowUpdates = false;
    if (timeId)
        killTimer(timeId);
    timeId = startTimer( 5000 );

    if (warningBox)
        delete warningBox;
}

#ifdef QTOPIA_PHONEUI
void PhoneLauncher::missedCount(int count)
{
    homeScreen->setMissedCalls(count);
    if (count != missedCallCount) {
        missedCallCount = count;
        showAlertDialogs();
    }
}

void PhoneLauncher::messageCountChanged(int count, bool full, bool fromSystem)
{
    alertedMessageCount = 0;
    messageBoxFull = full;
    isSysMsg = fromSystem;
    if (count != messageCount) {
        messageCount = count;
        homeScreen->setNewMessages(count);
        showAlertDialogs();
    }
}

void PhoneLauncher::messageRejected()
{
    showWarning(tr("Incoming Message Rejected"),
                tr("<qt>An incoming message was rejected because "
                   "there is insufficient space to store it.</qt>"));
}

void PhoneLauncher::activeCallCount(int count)
{
    activeCalls = count;
}

void PhoneLauncher::registrationChanged()
{

#ifdef QTOPIA_CELL
    QTelephony::RegistrationState r = cellModem->registrationState();
#endif

#ifdef QTOPIA_VOIP
    QTelephony::RegistrationState v = VoIPManager::instance()->registrationState();
#endif

    QString pix("antenna");

    QString cellMsg;
    QString voipMsg;

#ifdef QTOPIA_CELL
    bool roaming = false;
    switch (r) {
        case QTelephony::RegistrationNone:
            if (cellModem->planeModeEnabled()) {
                cellMsg = tr("Airplane safe mode");
                pix = "aeroplane";
            } else {
                cellMsg = tr("No network");
            }
            break;
        case QTelephony::RegistrationHome:
            // Nothing - this is the normal state
            break;
        case QTelephony::RegistrationSearching:
            cellMsg = tr("Searching for network");
            break;
        case QTelephony::RegistrationDenied:
            cellMsg = tr("Network registration denied");
            break;
        case QTelephony::RegistrationUnknown:
            //We can't do calls anyway
            cellMsg = tr("No network");
            break;
        case QTelephony::RegistrationRoaming:
            // ### probably want to beep/show message to let user know.
            roaming = true;
            break;
    }
#endif

#ifdef QTOPIA_VOIP
    switch (v) {
    case QTelephony::RegistrationNone:
        if(!voipHideMsg)
            voipMsg = tr("No VoIP network");
        break;
    case QTelephony::RegistrationHome:
    case QTelephony::RegistrationUnknown:
    case QTelephony::RegistrationRoaming:
        VoIPManager::instance()->startMonitoring();
        break;
    case QTelephony::RegistrationSearching:
        voipMsg = tr("Searching VoIP network");
        break;
    case QTelephony::RegistrationDenied:
        voipMsg += tr("VoIP Authentication Failed");
        break;
    }

    if(v == QTelephony::RegistrationNone) {
        if(!voipHideMsg && !voipHideMsgTimer)
            voipHideMsgTimer = startTimer(7000);
    } else {
        if(voipHideMsgTimer) {
            killTimer(voipHideMsgTimer);
            voipHideMsgTimer = 0;
        }
        voipHideMsg = false;
    }
#endif

    QString msg = cellMsg;
    if (!msg.isEmpty() && !voipMsg.isEmpty())
        msg += "<br>";
    msg.append(voipMsg);

    if (registrationMsgId) {
        homeScreen->clearInformation(registrationMsgId);
        registrationMsgId = 0;
    }

    if (!msg.isEmpty())
        registrationMsgId = homeScreen->showInformation(pix, msg);
}

#endif

void PhoneLauncher::timerEvent(QTimerEvent * tev)
{
    if(tev && tev->timerId() == updateTid) {
        update();
        killTimer(updateTid);
        updateTid = 0;
        return;
    }
#ifdef QTOPIA_VOIP
    else if(tev && tev->timerId() == voipHideMsgTimer) {
        voipHideMsg = true;
        killTimer(voipHideMsgTimer);
        voipHideMsgTimer = 0;
        registrationChanged();
        return;
    }
#endif

    if ( tev && multitaskingMultipressTimer.timerId() == tev->timerId() ) {
        multitaskingMultipressTimer.stop();
        return;
    }
}

#ifdef QTOPIA_PHONEUI
/*!
  Shows the Missed Calls dialog if there are any missed calls.  If there
  are no missed calls but there are new messages then the New Mesages
  dialog is shown.
*/
void PhoneLauncher::showAlertDialogs()
{
    if(homeScreen->keyLocked()) {
        showAlerts = true;
        return;
    }

    if (isVisible()) {
        if(DialerControl::instance()->missedCallCount() &&
           DialerControl::instance()->missedCallCount() != alertedMissed) {
            alertedMissed = DialerControl::instance()->missedCallCount();
            if (!missedMsgBox) {
                QString missedMsg = tr("Do you wish to view missed calls?");
                missedMsgBox = QAbstractMessageBox::messageBox(HomeScreen::getInstancePtr(), tr("Missed Call"),
                        "<qt>"+missedMsg+"</qt>", QAbstractMessageBox::Information,
                        QAbstractMessageBox::Yes, QAbstractMessageBox::No);
                connect(missedMsgBox, SIGNAL(finished(int)), this, SLOT(messageBoxDone(int)));
            }
            missedMsgBox->setTimeout(10000, QAbstractMessageBox::No);
            QtopiaApplication::showDialog(missedMsgBox);
            return;
        }

        if (messageCount &&
            messageCount != alertedMessageCount) {
            alertedMessageCount = messageCount;
            QString text;
            if (messageBoxFull) {
                text = tr("<qt>A new message has arrived and the incoming message box is full. Do you wish to read it now?</qt>");
            } else {
                text = tr("<qt>A new message has arrived. Do you wish to read it now?</qt>");
            }
            if (!messagesMsgBox) {
                messagesMsgBox = QAbstractMessageBox::messageBox(HomeScreen::getInstancePtr(), tr("New Message"), text,
                        QAbstractMessageBox::Information, QAbstractMessageBox::Yes, QAbstractMessageBox::No);
                connect(messagesMsgBox, SIGNAL(finished(int)), this, SLOT(messageBoxDone(int)));
            } else {
                messagesMsgBox->setText(text);
            }
            messagesMsgBox->setTimeout(10000, QAbstractMessageBox::No);
            QtopiaApplication::showDialog(messagesMsgBox);
        }
    }

    //    showAlertDialogs();
}
#endif

void PhoneLauncher::keyStateChanged(bool locked)
{
#ifdef QTOPIA_PHONEUI
    if(!locked)
        showAlertDialogs();
#else
    Q_UNUSED(locked);
#endif
}

void PhoneLauncher::updateBackground()
{
#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
    if (ThemeControl::instance()->exportBackground()) {

        ThemeItem *item = homeScreen->findItem("background", ThemedView::Item);
        bool wasExported = m_exportedBackground->isAvailable();
        if (item) {
            QDesktopWidget *desktop = QApplication::desktop();
            QRect desktopRect = desktop->screenGeometry(desktop->primaryScreen());
            QPixmap pm(desktopRect.width(),
                       desktopRect.height());
            QPainter p(&pm);
            QRect rect(QPoint(0,0), desktopRect.size());
            homeScreen->paint(&p, rect, item);
            QExportedBackground::setExportedBackground(pm);
        } else {
            QExportedBackground::clearExportedBackground();
        }

        if (!wasExported && m_exportedBackground->isAvailable())
            polishWindows();

    } else {
        QExportedBackground::clearExportedBackground();
    }
#endif
}


void PhoneLauncher::polishWindows()
{
    QApplication::setPalette(QApplication::palette());
    foreach (QWidget *w, QApplication::topLevelWidgets()) {
        QApplication::style()->polish(w);
        foreach (QObject *o, w->children()) {
            QWidget *sw = qobject_cast<QWidget*>(o);
            if (sw) {
                QApplication::style()->polish(sw);
            }
        }
    }
}

void PhoneLauncher::paintEvent(QPaintEvent *)
{
    QPixmap pm(":image/qpelogo");
    QPainter p(this);
    p.drawPixmap((width()-pm.width())/2,( height()-pm.height())/2, pm);
}

void PhoneLauncher::updateLauncherIconSize()
{
}

// Creation methods
PhoneHeader *PhoneLauncher::header()
{
    if(!m_header) {
        m_header = new PhoneHeader(0);
        WindowManagement::protectWindow(m_header);
        ThemeControl::instance()->registerThemedView(m_header, "Title");
        // Set width now to avoid relayout later.
        m_header->resize(QApplication::desktop()->screenGeometry().width(),
                        m_header->height());
    }

    return m_header;
}

void PhoneLauncher::createContext()
{
    Q_ASSERT(!m_context);

    m_context = new ContextLabel(0, Qt::FramelessWindowHint |
                                    Qt::Tool |
                                    Qt::WindowStaysOnTopHint );

    m_context->move(QApplication::desktop()->screenGeometry().topLeft()); // move to the correct screen
    WindowManagement::protectWindow(m_context);
    m_context->setAttribute(Qt::WA_GroupLeader);
    ThemeControl::instance()->registerThemedView(m_context, "Context");
    // Set width now to avoid relayout later.
    m_context->resize(QApplication::desktop()->screenGeometry().width(),
                    m_context->height());
}

ContextLabel *PhoneLauncher::context()
{
    Q_ASSERT(m_context);
    return m_context;
}

#ifdef QTOPIA_PHONEUI
void PhoneLauncher::showSpeedDialer(const QString &digits)
{
    showDialer(digits, true);
}

/*!
  Displays the dialer, preloaded with \a digits.  If \a speedDial
  is true, the digits will be considered for use in speed dial (press and
  hold).
*/
void PhoneLauncher::showDialer(const QString &digits, bool speedDial)
{
    if(speedDial) {
        dialer()->reset();
        dialer()->appendDigits(digits);
    } else {
        dialer()->setDigits(digits);
    }

    if ( callScreen() )
        dialer()->setGeometry( callScreen()->geometry() );

    dialer()->showMaximized();
    dialer()->raise();
}

/*!
  Displays the call history window.  If \a missed is true, the missed
  calls tab will be on top.  If \a hint is not empty, it is used to
  highlight a matching call.
*/
void PhoneLauncher::showCallHistory(bool missed, const QString &hint)
{
    if ( !callHistory() ) {
        mCallHistory = new CallHistory(DialerControl::instance()->callList(), 0);
        connect(callHistory(), SIGNAL(viewedMissedCalls()),
                this, SLOT(resetMissedCalls()) );
        connect(callHistory(), SIGNAL(viewedMissedCalls()),
                DialerControl::instance(), SLOT(resetMissedCalls()) );
        connect(callHistory(),
                SIGNAL(requestedDial(const QString&, const QUniqueId &)),
                this,
                SLOT(requestDial(const QString&, const QUniqueId &)));
    }
    callHistory()->reset();
    if (missed || DialerControl::instance()->missedCallCount() > 0)
        callHistory()->showMissedCalls();
    if( hint.length() )
        callHistory()->setFilter( hint );
    // update geometry, new theme may have been activated
    if ( callScreen() )
        callHistory()->setGeometry( callScreen()->geometry() );

    if( !callHistory()->isHidden() )
    {
        callHistory()->raise();
        callHistory()->refresh();
    }
    else
    {
        callHistory()->showMaximized();
    }
}
#endif

#ifdef QTOPIA_CELL

void PhoneLauncher::unstructuredNotification
    ( QSupplementaryServices::UnstructuredAction action, const QString& data )
{
    QString text;
    if ( !data.isEmpty() ) {
        text = data;
    } else {
        switch ( action ) {
            case QSupplementaryServices::TerminatedByNetwork:
                text = tr("Operation terminated by network");
                break;

            case QSupplementaryServices::OtherLocalClientResponded:
                text = tr("Other local client has responded");
                break;

            case QSupplementaryServices::OperationNotSupported:
                text = tr("Operation is not supported");
                break;

            case QSupplementaryServices::NetworkTimeout:
                text = tr("Operation timed out");
                break;

            default:
                text = tr("Network response: %1").arg((int)action);
                break;
        }
    }
    ussdMessage(text);
}

#endif // QTOPIA_CELL

#ifdef QTOPIA_PHONEUI
void PhoneLauncher::ussdMessage(const QString &text)
{
    QString title = tr("Service request");
    QString displayText = "<qt>" + text + "</qt>";
    if (!serviceMsgBox) {
        serviceMsgBox = QAbstractMessageBox::messageBox(0, title, displayText,
                                       QAbstractMessageBox::Information);
    } else {
        serviceMsgBox->setWindowTitle(title);
        serviceMsgBox->setText(displayText);
    }
    QtopiaApplication::showDialog(serviceMsgBox);
}
#endif

#ifdef QTOPIA_CELL

void PhoneLauncher::cellBroadcast(CellBroadcastControl::Type type,
                                  const QString &chan, const QString &text)
{
    if(type != CellBroadcastControl::Popup)
        return;

    // remove the previous message box if there is any
    // user will see the latest one only
    if (CBSMessageBox) {
        delete CBSMessageBox;
        CBSMessageBox = 0;
    }
    CBSMessageBox = QAbstractMessageBox::messageBox(HomeScreen::getInstancePtr(), chan,
                                   text, QAbstractMessageBox::Information,
                                   QAbstractMessageBox::No);
    QtopiaApplication::showDialog(CBSMessageBox);
}

#endif // QTOPIA_CELL

/*!
  Increases the earpiece volume.

  Note: this affects the user interface only.  Hardware specific code
  must be added to modify the actual earpiece volume.
*/
void PhoneLauncher::increaseEarVolume()
{
    if (!volumeScreen)
        volumeScreen = new EarpieceVolume();

    volumeScreen->increaseVolume();
}

/*!
  Decreases the earpiece volume.

  Note: this affects the user interface only.  Hardware specific code
  must be added to modify the actual earpiece volume.
*/
void PhoneLauncher::decreaseEarVolume()
{
    if (!volumeScreen)
        volumeScreen = new EarpieceVolume();

    volumeScreen->decreaseVolume();
}

/*!
  Shows the Profile settings application.
*/
void PhoneLauncher::showProfileSelector()
{
    QtopiaServiceRequest e( "Profiles", "showProfiles()" );
    e.send();
}


/*!
  Hides the Call History, Dialer and Call Screen.
*/
void PhoneLauncher::hideAll()
{
#ifdef QTOPIA_PHONEUI
    if (callScreen(false))
        callScreen(false)->close();
    if (callHistory())
        callHistory()->close();
    if (dialer(false))
        dialer(false)->close();
#endif
}

#ifdef QTOPIA_PHONEUI
void PhoneLauncher::stateChanged()
{
    callScreen()->stateChanged(); // We must know that this is updated before we continue

    if (DialerControl::instance()->hasIncomingCall()) {
        if ((callTypeMsgBox && callTypeMsgBox->isVisible())
#ifdef QTOPIA_VOIP
            || (voipNoPresenceMsgBox && voipNoPresenceMsgBox->isVisible())
#endif
            ) {
            // assume the request to call is cancelled
            queuedCall = QString();
            queuedCallType = QString();
            queuedCallContact = QUniqueId();
        }
        showCallScreen();
    }

    if (queuedIncoming && !DialerControl::instance()->hasIncomingCall()) {
        // incoming call has gone away
        queuedIncoming = false;
        delete incomingMsgBox;
        incomingMsgBox = 0;
    }

    if (queuedIncoming || !queuedCall.isEmpty()) {
        bool haveFg = DialerControl::instance()->hasActiveCalls();
        if (haveFg && !DialerControl::instance()->hasCallsOnHold()) {
            DialerControl::instance()->hold();
        } else if (!haveFg) {
            showCallScreen();
            if (queuedIncoming) {
                queuedIncoming = false;
                DialerControl::instance()->accept();
            } else {
                QString n(queuedCall);
                queuedCall = QString();
                queuedCallContact = QUniqueId();
                DialerControl::instance()->dial(n,DialerControl::instance()->callerIdNeeded(n), queuedCallType, queuedCallContact);
            }
        }
    }
}

void PhoneLauncher::acceptIncoming()
{
    if (DialerControl::instance()->hasIncomingCall()) {
        showCallScreen();
        if(DialerControl::instance()->hasActiveCalls() &&
           DialerControl::instance()->hasCallsOnHold()) {
            if (!incomingMsgBox) {
                incomingMsgBox = QAbstractMessageBox::messageBox(
                    callScreen(),
                    tr("End current call?"),
                    tr("<qt>Do you wish to end the current call before answering the incoming call?</qt>"),
                    QAbstractMessageBox::Warning, QAbstractMessageBox::Yes, QAbstractMessageBox::No);
                connect(incomingMsgBox, SIGNAL(finished(int)), this, SLOT(messageBoxDone(int)));
            }
            QtopiaApplication::showDialog(incomingMsgBox);
            queuedIncoming = true;
        } else {
            DialerControl::instance()->accept();
        }
    }
}

/*!
  Accepts an incoming call.  Has no affect if an incoming call is not
  currently available.
*/
void PhoneLauncher::messageBoxDone(int r)
{
    QAbstractMessageBox *box = (QAbstractMessageBox*)sender();
    if (box == missedMsgBox && r == QAbstractMessageBox::Yes) {
        showCallHistory(true);
    } else if (box == messagesMsgBox) {
        // stop message ring tone
        RingControl *rc = qtopiaTask<RingControl>();
        if (rc) rc->stopRing();
        if ( r == QAbstractMessageBox::Yes) {
            QtopiaServiceRequest req;
            req.setService("SMS");

            if ( !isSysMsg )
                req.setMessage("viewSms()");
            else
                req.setMessage("viewSysSms()");

            req.send();
        }
    } else if (box == dialingMsgBox) {
        if (r == QAbstractMessageBox::Yes)
            DialerControl::instance()->endCall();
        else
        {
            queuedCall = QString();
            queuedCallContact = QUniqueId();
        }
    } else if (box == incomingMsgBox) {
        if (r == QAbstractMessageBox::Yes) {
            DialerControl::instance()->endCall();
        } else {
            queuedIncoming = false;
            if (DialerControl::instance()->hasIncomingCall())
                DialerControl::instance()->incomingCall().hangup();
        }
    } else if (box == callTypeMsgBox) {
        switch(r) {
        case 0:
            queuedCallType = "Voice";     // No tr
            dialNumber(queuedCall, queuedCallContact, queuedCallType);
            break;
        case 1:
            queuedCallType = "VoIP";      // No tr
#ifdef QTOPIA_VOIP
            if ( VoIPManager::instance()->isAvailable( queuedCall ) ) {
                dialNumber(queuedCall, queuedCallContact, queuedCallType);
            } else {
                if (!voipNoPresenceMsgBox) {
                    voipNoPresenceMsgBox = QAbstractMessageBox::messageBox(callScreen(), tr("Unavailable"),
                        tr("<qt>The selected contact appears to be unavailable. Do you still wish to make a call?</qt>"),
                        QAbstractMessageBox::Warning, QAbstractMessageBox::Yes, QAbstractMessageBox::No);
                    connect(voipNoPresenceMsgBox, SIGNAL(finished(int)), this, SLOT(messageBoxDone(int)));
                }
                QtopiaApplication::showDialog(voipNoPresenceMsgBox);
            }
#endif
            break;
        default:
            break;
        }
    }
#ifdef QTOPIA_VOIP
    else if (box == voipNoPresenceMsgBox) {
        if (r == QAbstractMessageBox::Yes) {
            dialNumber(queuedCall, queuedCallContact, queuedCallType);
        } else {
            queuedCall = QString();
            queuedCallType = QString();
            queuedCallContact = QUniqueId();
        }
    }
#endif
}
#endif

void PhoneLauncher::showWarning(const QString &title, const QString &text)
{
    if (!warningMsgBox)
        warningMsgBox = QAbstractMessageBox::messageBox(0, title, text, QAbstractMessageBox::Warning);
    warningMsgBox->setWindowTitle(title);
    warningMsgBox->setText(text);
    warningMsgBox->setTimeout(3000, QAbstractMessageBox::NoButton);
    QtopiaApplication::showDialog(warningMsgBox);
}

#ifdef QTOPIA_PHONEUI
/*!
  Dials number \a n.  The current call state is first checked to confirm
  that it is possible to make a call.  If it is not possible to make a
  call, the user will be notified, otherwise the call screen will be shown
  and the call initiated.
*/
void PhoneLauncher::dialNumber(const QString &n, const QUniqueId &c, const QString &callType)
{
    if (callType.isEmpty() || n.isEmpty())
        return;

    showCallScreen();

    if (DialerControl::instance()->hasActiveCalls() && DialerControl::instance()->hasCallsOnHold()) {
        if (!dialingMsgBox) {
            dialingMsgBox = QAbstractMessageBox::messageBox(
                    callScreen(),
                    tr("End current call?"),
                    tr("<qt>Do you wish to end the current call before begining the new call?</qt>"),
                    QAbstractMessageBox::Warning, QAbstractMessageBox::Yes, QAbstractMessageBox::No);
             connect(dialingMsgBox, SIGNAL(finished(int)), this, SLOT(messageBoxDone(int)));
        }
        QtopiaApplication::showDialog(dialingMsgBox);
    } else if (DialerControl::instance()->hasActiveCalls()) {
        DialerControl::instance()->hold();
    } else {
        DialerControl::instance()->dial( n, DialerControl::instance()->callerIdNeeded(n), callType, c );
    }
}

void PhoneLauncher::requestDial(const QString &n, const QUniqueId &c)
{
    if (n.isEmpty())
        return;

#ifdef QTOPIA_VOIP
    bool voipReg = VoIPManager::instance()->registrationState() == QTelephony::RegistrationHome;
#endif
#ifdef QTOPIA_CELL
    bool gsmReg = cellModem->networkRegistered();
#endif

#if defined (QTOPIA_VOIP) && defined (QTOPIA_CELL)
    if (!gsmReg && !voipReg) {
        showWarning(tr("No GSM/VoIP Network"),
                tr("<qt>No phone call is possible.</qt>"));
        return;
    }
#endif

    queuedCall = n;
    queuedCallType = QString();
    queuedCallContact = c;

#ifdef QTOPIA_CELL
    if(cellModem->planeModeEnabled()) {
        // At the moment, plane mode is the only way to turn
        // the GSM module off.
        showWarning(tr("Airplane Mode"),
            tr("<qt>Phone calls cannot be made in Airplane Mode.</qt>"));
        return;
    }
#endif

    queuedCallType = "Voice";     // No tr

#ifdef QTOPIA_VOIP
    bool anyType = false;
    int numberAtSymbol = n.count('@');
    if (numberAtSymbol == 1) {
        queuedCallType = "VoIP";          // No tr
    }
    if (numberAtSymbol > 1) {
        showWarning(tr("Incorrect Number format"),
                tr("<qt>Unable to make a phone call.</qt>"));
        return;
    }
    if (numberAtSymbol == 0) {
        // its a pure number can go on any line
        anyType = true;
    }

#ifndef QTOPIA_CELL
    // No GSM network, so assume that everything is a VoIP call.
    queuedCallType = "VoIP";      // No tr
#else
    if (CellModemManager::emergencyNumbers().contains(n)) {
        // Emergency numbers should always be dialed with GSM,
        // as the VoIP provider probably doesn't have support for it.
        queuedCallType = "Voice";     // No tr
    } else if (anyType) {
        if (gsmReg && !voipReg) { // only gsm network available
            queuedCallType = "Voice";
        } else if (!gsmReg && voipReg) { // only voip network available
            queuedCallType = "VoIP";
        } else { // both available
            if (!callTypeMsgBox) {
                callTypeMsgBox = QAbstractMessageBox::messageBoxCustomButton(callScreen(), tr("GSM or VoIP call?"),
                    tr("<qt>Do you wish to make a GSM or a VoIP Call?</qt>"),
                    QAbstractMessageBox::Warning, tr("GSM"), tr("VoIP"), QString(), 0);
                connect(callTypeMsgBox, SIGNAL(finished(int)), this, SLOT(messageBoxDone(int)));
            }
            QtopiaApplication::showDialog(callTypeMsgBox);
            return;
        }
    }
#endif
    if (queuedCallType == "VoIP" && !VoIPManager::instance()->isAvailable(queuedCall)) {
        if (!voipNoPresenceMsgBox) {
            voipNoPresenceMsgBox = QAbstractMessageBox::messageBox(callScreen(), tr("Unavailable"),
                tr("<qt>The selected contact appears to be unavailable. Do you still wish to make a call?</qt>"),
                QAbstractMessageBox::Warning, QAbstractMessageBox::Yes, QAbstractMessageBox::No);
            connect(voipNoPresenceMsgBox, SIGNAL(finished(int)), this, SLOT(messageBoxDone(int)));
        }
        QtopiaApplication::showDialog(voipNoPresenceMsgBox);
        return;
    }
#endif
    dialNumber(queuedCall, queuedCallContact, queuedCallType);
}

void PhoneLauncher::serviceNumber(QServiceNumbers::NumberId id, const QString& number)
{
    if ( id == QServiceNumbers::VoiceMail ) {
        // The "waitingVoiceMailNumber" flag is used to prevent
        // false positives when the "phonesettings" program
        // queries for the voice mail number.
        if ( waitingVoiceMailNumber ) {
            waitingVoiceMailNumber = false;
            if ( number.length() > 0 ) {
                requestDial( number );
            } else {
                showWarning(tr("Voice Mail"),
                            tr("<qt>Voice mail number is not set.</qt>"));
            }
        }
    }
}

/*!
    Dial the voice mail number in the SIM.
*/
void PhoneLauncher::dialVoiceMail()
{
    waitingVoiceMailNumber = true;
    if ( serviceNumbers->available() )
        serviceNumbers->requestServiceNumber( QServiceNumbers::VoiceMail );
    else
        serviceNumber( QServiceNumbers::VoiceMail, QString() );
}

void PhoneLauncher::resetMissedCalls()
{
    alertedMissed = 0;
}


QAbstractDialerScreen *PhoneLauncher::dialer(bool create) const
{
    if(create && !m_dialer) {
        m_dialer = qtopiaWidget<QAbstractDialerScreen>(0);
        m_dialer->move(QApplication::desktop()->screenGeometry().topLeft());

        connect(m_dialer,
                SIGNAL(requestDial(const QString&, const QUniqueId&)),
                this,
                SLOT(requestDial(const QString&, const QUniqueId&)));
        connect(m_dialer, SIGNAL(speedDial(const QString&)),
                this, SLOT(speedDial(const QString&)) );
    }

    return m_dialer;
}

CallScreen *PhoneLauncher::callScreen(bool create) const
{
    if(create && !mCallScreen) {
        mCallScreen = new CallScreen(DialerControl::instance(), 0);
        mCallScreen->move(QApplication::desktop()->screenGeometry().topLeft());
        QObject::connect(mCallScreen, SIGNAL(increaseCallVolume()),
                         this, SLOT(increaseEarVolume()));
        QObject::connect(mCallScreen, SIGNAL(decreaseCallVolume()),
                         this, SLOT(decreaseEarVolume()));
        RingControl *rc = qtopiaTask<RingControl>();
        if(rc)
            QObject::connect(mCallScreen, SIGNAL(muteRing()),
                             rc, SLOT(muteRing()));
        QObject::connect(mCallScreen, SIGNAL(listEmpty()),
                         mCallScreen, SLOT(hide()));
        QObject::connect(mCallScreen, SIGNAL(acceptIncoming()),
                         this, SLOT(acceptIncoming()));
        ThemeControl::instance()->registerThemedView(mCallScreen, "CallScreen");
    }
    return mCallScreen;

}
#endif

void PhoneLauncher::speedDial( const QString& input )
{
    activateSpeedDial(input);
#ifdef QTOPIA_PHONEUI
    if ( dialer(false) )
        dialer(false)->reset();
#endif
}

/*!
  Activate speed dial associated with \a input.
 */
bool PhoneLauncher::activateSpeedDial( const QString& input )
{
    if ( !speeddialfeedback )
        speeddialfeedback = new QSpeedDialFeedback;
    QDesktopWidget *desktop = QApplication::desktop();
    QtopiaServiceDescription* r = QSpeedDial::find(input);
    if(r)
    {
        speeddialfeedback->show(desktop->screen(desktop->primaryScreen()),input,*r);
        return !r->request().isNull();
    }
    else
    {
        speeddialfeedback->show(desktop->screen(desktop->primaryScreen()),input,QtopiaServiceDescription());
        return false;
    }
}


QAbstractBrowserScreen *PhoneLauncher::phoneBrowser(bool create) const
{
    if(!stack && create) {
        stack = qtopiaWidget<QAbstractBrowserScreen>();
        stack->move(QApplication::desktop()->screenGeometry().topLeft());
    }

    return stack;
}

QAbstractSecondaryDisplay *PhoneLauncher::secondaryDisplay(bool create) const
{
    QDesktopWidget *desktop = QApplication::desktop();
    if (!secondDisplay && create && desktop->numScreens() > 1) {
        secondDisplay = qtopiaWidget<QAbstractSecondaryDisplay>(0,
                                            Qt::FramelessWindowHint | Qt::Tool);
        secondDisplay->setGeometry(desktop->screenGeometry(1));
        secondDisplay->show();
    }

    return secondDisplay;
}

// define MultiTaskProxy
MultiTaskProxy::MultiTaskProxy(QObject *parent)
: TaskManagerService(parent)
{
}

void MultiTaskProxy::multitask()
{
    emit doMultiTask();
}

void MultiTaskProxy::showRunningTasks()
{
    emit doShowRunningTasks();
}

// define EarpieceVolume
EarpieceVolume::EarpieceVolume(QWidget *parent, Qt::WFlags f)
    : QWidget(parent, f), tid(0), minLevel(0), maxLevel(5), level(3)
{
    //XXX read volume min/max from somewhere.

    setWindowTitle(tr("Volume"));
    QGridLayout *gl = new QGridLayout(this);//, 5, 2, 8, 8);
    QLabel *icon = new QLabel(this);
    icon->setPixmap(QIcon(":icon/volume").pixmap(QSize(14,14)));
    gl->addWidget(icon, 1, 0);
    label = new QLabel(this);
    label->setText(tr("<b>Ear Volume: %1</b>").arg(level));
    gl->addWidget(label, 1, 1);
    slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(minLevel, maxLevel);
    slider->setValue(level);
    slider->setTickPosition(QSlider::TicksBelow);
    slider->installEventFilter(this);
    QSoftMenuBar::setLabel(slider, Qt::Key_Select, QSoftMenuBar::NoLabel);
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(levelChanged(int)));
    gl->addWidget(slider, 2, 0, 1, 2);
    QLabel *l = new QLabel(QString::number(minLevel), this);
    gl->addWidget(l, 3, 0);
    l = new QLabel(QString::number(maxLevel), this);
    l->setAlignment(Qt::AlignRight);
    gl->addWidget(l, 3, 1);
    gl->setRowStretch(0, 1);
    gl->setRowStretch(4, 1);
    gl->setColumnStretch(1, 1);
}

void EarpieceVolume::increaseVolume()
{
    if (!isVisible()) {
        showMaximized();
        if (tid)
            killTimer(tid);
        tid = startTimer(3000);
    }
    if (++level > maxLevel)
        level = maxLevel;
    else
        slider->setValue(level);
}

void EarpieceVolume::decreaseVolume()
{
    if (!isVisible()) {
        showMaximized();
        if (tid)
            killTimer(tid);
        tid = startTimer(3000);
    }
    if (--level < minLevel)
        level = minLevel;
    else
        slider->setValue(level);
}

void EarpieceVolume::levelChanged(int v)
{
    level = v;
    label->setText(tr("<b>Ear Volume: %1</b>").arg(level));
    //XXX set hardware volume.
    if (tid)
        killTimer(tid);
    tid = startTimer(3000);
}

void EarpieceVolume::timerEvent(QTimerEvent *)
{
    killTimer(tid);
    tid = 0;
    close();
}

bool EarpieceVolume::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = (QKeyEvent*)e;
        if (ke->key() == Qt::Key_Select) {
            return true;
        } else if (ke->key() == Qt::Key_Back) {
            // Don't want to cancel setting.
            close();
            return true;
        }
    }

    return false;
}

// define QSpeedDialFeedback
QSpeedDialFeedback::QSpeedDialFeedback() :
    QFrame(0, (Qt::Tool | Qt::FramelessWindowHint)),
    timerId(0)
{
    setFrameStyle(QFrame::WinPanel|QFrame::Raised);
    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->setMargin(8);
    icon = new QLabel(this);
    vb->addWidget(icon);
    label = new QLabel(this);
    vb->addWidget(label);
    icon->setAlignment(Qt::AlignCenter);
    label->setAlignment(Qt::AlignCenter);
}

void QSpeedDialFeedback::show(QWidget* center, const QString& input, const QtopiaServiceDescription& r)
{
    req = r.request();
#ifdef QTOPIA_PHONEUI
    RingControl *rc = qtopiaTask<RingControl>();
    if ( req.isNull() ) {
        if(rc) rc->playSound(":sound/speeddial/nak");
    } else {
        if(rc) rc->playSound(":sound/speeddial/ack");
    }
#endif
    if ( req.isNull() ) {
        icon->setPixmap(QPixmap(":icon/cancel"));
        label->setText(tr("No speed dial %1").arg(input));
    } else {
        icon->setPixmap(QPixmap(":image/"+r.iconName()));
        label->setText(r.label());
    }
    QtopiaApplication::sendPostedEvents(this, QEvent::LayoutRequest);
    QSize sh = sizeHint();
    QRect w = center->topLevelWidget()->geometry();
    //We have to set the minimumsize before we change the geometry
    //because setGeometry is based on it and for some weired reason
    //Minimumsize is set to the size of the previous geometry.
    //This is a problem when changing from a bigger to a smaller geometry
    //and seems to be a bug in Qt.
    setMinimumSize(sh.width(),sh.height());
    setGeometry(w.x()+(w.width()-sh.width())/2,
                w.y()+(w.height()-sh.height())/2,sh.width(),sh.height());
    QFrame::show();
    activateWindow();
    setFocus();
    if( Qtopia::mousePreferred() ) {
        if ( !req.isNull() )
            req.send();
        timerId = startTimer(1000);
    }
}

void QSpeedDialFeedback::timerEvent(QTimerEvent*)
{
    killTimer(timerId);
    close();
}

void QSpeedDialFeedback::keyReleaseEvent(QKeyEvent* ke)
{
    if ( !ke->isAutoRepeat() ) {
        if ( !req.isNull() )
            req.send();
        close();
    }
}

void QSpeedDialFeedback::mouseReleaseEvent(QMouseEvent*)
{
    if ( !req.isNull() )
        req.send();
    close();
}

QTOPIA_REPLACE_WIDGET(QAbstractServerInterface, PhoneLauncher);

#include "phonelauncher.moc"
