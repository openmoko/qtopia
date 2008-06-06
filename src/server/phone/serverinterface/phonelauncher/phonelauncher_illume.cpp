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

#include <qtopiaapplication.h>

#include "qtopiaserverapplication.h"
#include <qtopiaipcenvelope.h>
#include <qtopiaipcadaptor.h>
#include <QtopiaItemDelegate>
#include <qtranslatablesettings.h>
#include <qtopiaservices.h>
#include <qdocumentproperties.h>
#include <qcategoryselector.h>
#include <qsoftmenubar.h>
#include <qtopianamespace.h>
#include <qtopiaservices.h>
#include <qtopialog.h>
#include <qtopianamespace.h>
#include <custom.h>
#include <themedview.h>
#include <qdrmcontent.h>
#include <QPhoneProfile>
#include <QPhoneProfileManager>
#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
#include <QExportedBackground>
#else
#include <QGlobalPixmapCache>
#endif
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
#include "windowmanagement.h"
#include "contextlabel.h"
#include "messagebox.h"

#include "contentserver.h"
#include "phonebrowser.h"
#include "homescreencontrol.h"
#include "qabstracthomescreen.h"
#include "documentview.h"
#include "alarmcontrol.h"
#include "taskmanagerservice.h"
#include "themecontrol.h"
#include "qabstractsecondarydisplay.h"
#include "contentsetlauncherview.h"
#include "themebackground_p.h"
#include "lowmemorytask.h"
#include "qtopiainputevents.h"
#include "qabstractcallpolicymanager.h"

#ifdef QTOPIA_VOIP
#include "voipmanager.h"
#endif

#ifdef QTOPIA_PHONEUI
#include "ringcontrol.h"
#include "qabstractdialerscreen.h"
#include "dialercontrol.h"
#include "dialerservice.h"
#include "callscreen.h"
#if defined(QTOPIA_TELEPHONY)
#include "callhistory.h"
#endif
#include "dialercontrol.h"
#include "messagecontrol.h"
#endif

#ifdef QTOPIA_CELL
#include "gsmkeyactions.h"
#endif

#include "simpindialog.h"
#include "contactdbusexporter.h"
#include "phonestatusdbusexporter.h"
#include "profiledbusexporter.h"
#include "smsstatusdbusexporter.h"

#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

static const int NotificationVisualTimeout = 0;  // e.g. New message arrived, 0 == No timeout
static const int WarningTimeout = 5000;  // e.g. Cannot call

class RejectDlg : public QDialog
{
public:
    void rejectDlg() { reject(); }
    void hideDlg() { hide(); }
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

/*!
  \class PhoneLauncher
  \brief The PhoneLauncher class implments the main UI widget for the Qtopia Phone Edition.
  \ingroup QtopiaServer::PhoneUI

  This class is a Qtopia \l{QtopiaServerApplication#qtopia-server-widgets}{server widget}. 
  It is part of the Qtopia server and cannot be used by other Qtopia applications.

  \sa QAbstractServerInterface
  */

/*!
  \internal
  */
PhoneLauncher::PhoneLauncher(QWidget *parent, Qt::WFlags fl)
    : QAbstractServerInterface(parent, fl), m_updateTid(0),
      m_context(0), m_stack(0), m_homeScreen(0),
      m_registrationMessageId(0),
    m_warningMessageBox(0),
#ifdef QTOPIA_PHONEUI
      m_messageCount(0), m_activeCalls(0),
      m_newMessages("Communications/Messages/NewMessages"),
    m_serviceMessageBox(0), m_CBSMessageBox(0),
    m_callScreen(0), m_dialer(0),
    m_callHistory(0),
    m_missedMessageBox(0), m_messagesMessageBox(0), m_incomingMessageBox(0),
    m_dialingMessageBox(0), m_callTypeMessageBox(0),
    m_alertedMissed(0),
    m_messageBoxFull(false),
    m_isSystemMessage(false),
    m_queuedIncoming(false),
    m_waitingVoiceMailNumber(false),
#endif
#ifdef QTOPIA_VOIP
    m_voipNoPresenceMessageBox(0),
#endif
#ifdef QTOPIA_CELL
    m_cellModem(0), m_gsmKeyActions(0)
#endif
{
    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect = desktop->screenGeometry(desktop->primaryScreen());
#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
    QExportedBackground::initExportedBackground(desktopRect.width(),
                                                desktopRect.height(),
                                                desktop->primaryScreen());
#else
    {
        //Needed to initialise cache!
        QPixmap pm;
        if(!QGlobalPixmapCache::find("qtopia", pm)) {
            pm.load("qtopia.png");
            QGlobalPixmapCache::insert("qtopia", pm);
        }
    }

#endif
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
#ifdef QT_ILLUME_LAUNCHER
    setGeometry(0, 0, 0, 0);
    hide();
#else
    setGeometry(desktopRect);
#endif

    QObject::connect(ThemeControl::instance(), SIGNAL(themeChanged()),
                     this, SLOT(loadTheme()));

    // Create phone context bar
    createContext();

    // Create home screen
    m_homeScreen = qtopiaWidget<QAbstractHomeScreen>(this);
    // Homescreen covers the entire screen
#ifdef QT_ILLUME_LAUNCHER
    m_homeScreen->setGeometry(0, 0, 0, 0);
    m_homeScreen->hide();
#else
    if (m_homeScreen->geometry() != rect())
        m_homeScreen->setGeometry(0, 0, width(), height());
#endif
    HomeScreenControl::instance()->setHomeScreen(m_homeScreen);

    QObject::connect(m_homeScreen, SIGNAL(keyLockedChanged(bool)),
                     this, SLOT(keyStateChanged(bool)));
#ifdef QTOPIA_PHONEUI
    QObject::connect(m_homeScreen, SIGNAL(showCallScreen()),
                     this, SLOT(showCallScreen()));
    QObject::connect(m_homeScreen, SIGNAL(showMissedCalls()),
                     this, SLOT(showMissedCalls()));
    QObject::connect(m_homeScreen, SIGNAL(showCallHistory()),
                     this, SLOT(showCallHistory()));
    QObject::connect(m_homeScreen, SIGNAL(callEmergency(QString)),
                    this, SLOT(requestDial(QString)));
#endif


#ifdef QTOPIA_PHONEUI
    // Implement dialer service
    DialerServiceProxy *dialerServiceProxy = new DialerServiceProxy(this);
    connect(dialerServiceProxy, SIGNAL(doDialVoiceMail()),
            this, SLOT(dialVoiceMail()));
    connect(dialerServiceProxy, SIGNAL(doDial(QString)),
            this, SLOT(requestDial(QString)));
    connect(dialerServiceProxy, SIGNAL(doDial(QString,QUniqueId)),
            this, SLOT(requestDial(QString,QUniqueId)));
    connect(dialerServiceProxy, SIGNAL(doShowDialer(QString)),
            this, SLOT(showDialer(QString)));
#endif

    // Listen to system channel
    QtopiaChannel* sysChannel = new QtopiaChannel( "QPE/System", this );
    connect( sysChannel, SIGNAL(received(QString,QByteArray)),
             this, SLOT(sysMessage(QString,QByteArray)) );

    showHomeScreen(0);
    m_homeScreen->setFocus();

#ifdef QTOPIA_PHONEUI
    connect(DialerControl::instance(), SIGNAL(missedCount(int)),
            this, SLOT(missedCount(int)));
    connect(MessageControl::instance(), SIGNAL(messageCount(int,bool,bool,bool)),
            this, SLOT(messageCountChanged(int,bool,bool,bool)));
    connect(MessageControl::instance(), SIGNAL(smsMemoryFull(bool)),
            this, SLOT(smsMemoryFull(bool)));
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
    m_cellModem = qtopiaTask<CellModemManager>();
    connect(m_cellModem, SIGNAL(planeModeEnabledChanged(bool)),
            this, SLOT(registrationChanged()));
    m_gsmKeyActions = new GsmKeyActions(this);
#endif

#if defined(QTOPIA_TELEPHONY)
    // Hook onto registration state changes for all call policy managers.
    // The standard ones are CellModemManager and VoIPManager.
    QList<QAbstractCallPolicyManager *> managers;
    managers = qtopiaTasks<QAbstractCallPolicyManager>();
    foreach (QAbstractCallPolicyManager *manager, managers) {
        connect(manager,
                SIGNAL(registrationChanged(QTelephony::RegistrationState)),
                this,
                SLOT(registrationChanged()));
    }
#endif

#ifdef QTOPIA_PHONEUI
    // Voice mail
    m_serviceNumbers = new QServiceNumbers( QString(), this );
    connect( m_serviceNumbers, SIGNAL(serviceNumber(QServiceNumbers::NumberId,QString)),
             this, SLOT(serviceNumber(QServiceNumbers::NumberId,QString)) );

    m_homeScreen->setNewMessages(m_newMessages.value().toInt());

    connect(&m_newMessages, SIGNAL(contentsChanged()),
            this, SLOT(newMessagesChanged()));

    // Don't alert user until count changes.
    m_alertedMissed = DialerControl::instance()->missedCallCount();
#endif
//    ThemeControl::instance()->refresh();

#ifdef QTOPIA_PHONEUI
    registrationChanged();
#endif

#ifdef QT_ILLUME_LAUNCHER
    SimPinDialog* dialog = new SimPinDialog(this);
    QDBusConnection::sessionBus().registerObject("/Contacts",
                                                 new ContactDBusExporter(this),
                                                 QDBusConnection::ExportScriptableContents);
    QDBusConnection::sessionBus().registerObject("/Status",
                                                 new PhoneStatusDBusExporter(dialog, this),
                                                 QDBusConnection::ExportScriptableContents);
    QDBusConnection::sessionBus().registerObject("/Sms",
                                                 new SMSStatusDBusExporter(this),
                                                 QDBusConnection::ExportScriptableContents);
    QDBusConnection::sessionBus().registerObject("/Profiles",
                                                 new ProfileDBusExporter(this),
                                                 QDBusConnection::ExportScriptableContents);
    QDBusConnection::sessionBus().registerService("org.openmoko.qtopia.Phonestatus");
#endif

    loadTheme();
}

/*!
  \internal
  */
PhoneLauncher::~PhoneLauncher()
{
    delete m_stack;
    delete m_context;
#ifdef QTOPIA_PHONEUI
    delete m_callScreen;
    delete m_dialer;
#endif
#if defined(QTOPIA_TELEPHONY)
    delete m_callHistory;
#endif
}

/*!
  \internal
  */
void PhoneLauncher::showEvent(QShowEvent *e)
{
#ifndef QT_ILLUME_LAUNCHER
    QTimer::singleShot(0, m_homeScreen, SLOT(show()));
    QTimer::singleShot(0, m_homeScreen, SLOT(applyHomeScreenImage()));
#endif

    QTimer::singleShot(0, this, SLOT(updateBackground()));
#ifdef QTOPIA_PHONEUI
    QTimer::singleShot(0, this, SLOT(initializeCallHistory()));
#endif

    if (QSoftMenuBar::keys().count())
        context()->show();
    QWidget::showEvent(e);
}

/*!
  \internal
  */
void PhoneLauncher::callPressed()
{
#ifdef QTOPIA_PHONEUI
    // Called if server windows are not on top
    showCallHistory();
#endif
}

/*!
  \internal
  */
void PhoneLauncher::loadTheme()
{
    bool v = isVisible();

    qLog(UI) << "Load theme";
    m_homeScreen->hide();

    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect = desktop->screenGeometry(desktop->primaryScreen());

    // context bar - not lazy
    WindowManagement::dockWindow(context(), WindowManagement::Bottom, context()->reservedSize());
    if (v && QSoftMenuBar::keys().count())
        context()->show();

    // home screen - not lazy
#ifdef QT_ILLUME_LAUNCHER
    m_homeScreen->setGeometry(0, 0, 0, 0);
#else
    m_homeScreen->setGeometry(desktopRect);
#endif
    m_homeScreen->updateInformation();

#ifdef QTOPIA_PHONEUI
    // call screen - lazy
    CallScreen *cs = callScreen(false);
    if(cs) {
        cs->setGeometry(desktopRect.x(), desktopRect.y(),
                        desktopRect.width(),
                        desktopRect.height() - context()->height() );
    }
#endif

    initInfo();

    if ( v ) {
        QTimer::singleShot(0, m_homeScreen, SLOT(show()));
        QTimer::singleShot(0, this, SLOT(updateBackground()));
    }
}

#ifdef QTOPIA_PHONEUI
/*!
  \internal
  */
void PhoneLauncher::showMissedCalls()
{
    showCallHistory(true);
}

/*!
  \internal
  Displays the call screen.
*/
void PhoneLauncher::showCallScreen()
{
    if( !callScreen()->sourceLoaded() ) {
        callScreen()->loadSource();
    }
    callScreen()->showMaximized();
    callScreen()->raise();
    callScreen()->activateWindow();
}
#endif

/*!
  \internal
  */
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

/*!
  \internal
  */
void PhoneLauncher::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
}

/*!
  \internal
  */
void PhoneLauncher::closeEvent(QCloseEvent *e)
{
    e->ignore();
}

/*!
  \internal
  */
void PhoneLauncher::sysMessage(const QString& message, const QByteArray &data)
{
    QDataStream stream( data );
    if ( message == "showHomeScreen()" ) {
        showHomeScreen(0);
    } else if ( message == QLatin1String("showHomeScreenAndToggleKeylock()") ) {
        showHomeScreen(2);
    } else if ( message == QLatin1String("showHomeScreenAndKeylock()") ) {
        showHomeScreen(3);
    } else if ( message == "applyStyleSplash()" ) {
        raise();
        qApp->processEvents();
        ThemeControl::instance()->refresh();
        polishWindows();
        updateLauncherIconSize();
        m_homeScreen->applyHomeScreenImage();
        lower();
    } else if ( message == "applyStyleNoSplash()" ) {
        qApp->processEvents();
        ThemeControl::instance()->refresh();
        polishWindows();
        updateLauncherIconSize();
        m_homeScreen->applyHomeScreenImage();
    } else if ( message == "applyHomeScreenImage()" ) {
        m_homeScreen->applyHomeScreenImage();
        updateBackground();
    } else if ( message == "updateHomeScreenInfo()" ) {
        m_homeScreen->updateHomeScreenInfo();
    } else if ( message == "serverKey(int,int)" ) {
        int key,press;
        stream >> key >> press;
        if ( key == Qt::Key_Call && press ) {
            callPressed();
        }
    }
}

void PhoneLauncher::showHomeScreen(int state)
{
    // state: 0 -> no screensaver calls
    //        1 -> showHomeScreen called by screensaver
    //        2 -> showHomeScreen called when lock key is toggled
    //        3 -> showHomeScreen called to key lock

#ifdef QTOPIA_PHONEUI
    if (state != 0 && m_activeCalls) {
        return;
    }
#endif

    rejectModalDialog();
    topLevelWidget()->raise();
    m_homeScreen->raise();
    m_homeScreen->show();
    m_homeScreen->setFocus();
    topLevelWidget()->activateWindow();

    if (state == 1) {
        //QtopiaIpcEnvelope closeApps( "QPE/System", "close()" );
        if (!Qtopia::mousePreferred()) {
            QSettings c("Trolltech","qpe");
            c.beginGroup("HomeScreen");
            QString lockType = c.value( "AutoKeyLock", "Disabled" ).toString();
            if (lockType == "Enabled")
                m_homeScreen->setKeyLocked(true);
        }
    } else if (state == 2 || state == 3) {
        if (!m_homeScreen->keyLocked())
            m_homeScreen->setKeyLocked(true);
        else if (state == 2)
            QtopiaInputEvents::processKeyEvent(0, BasicKeyLock::lockKey(), 0, true, false);
    }
}

/*!
  \internal
  */
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

#ifdef QTOPIA_PHONEUI
/*!
  \internal
  */
void PhoneLauncher::missedCount(int count)
{
    m_homeScreen->setMissedCalls(count);

    if(m_homeScreen->keyLocked()
#ifdef QTOPIA_CELL
        || m_homeScreen->simLocked()
#endif
    ){
        // do not show alert this time
        // but reset missed count so when the phone is unlocked the alert is shown.
        resetMissedCalls();
    } else {
        showAlertDialogs();
    }
}

/*!
  \internal
  */
void PhoneLauncher::messageCountChanged(int count, bool full, bool fromSystem, bool notify)
{ 
    m_messageBoxFull = full;
    m_isSystemMessage = fromSystem;
    if (count != m_messageCount) {
        m_messageCount = count;
        if (notify && m_messageCount) {
            QtopiaServiceRequest req("Messages","viewNewMessages(bool)");
            req << false;
            req.send();
        }
    }
}

/*!
  \internal
  */
void PhoneLauncher::smsMemoryFull(bool full)
{
    m_homeScreen->setSmsMemoryFull(full);
}

/*!
  \internal
  */
void PhoneLauncher::messageRejected()
{
    showWarning(tr("Message Rejected"),
                tr("<qt>An incoming message was rejected because "
                   "there is insufficient space to store it.</qt>"));
}

/*!
  \internal
  */
void PhoneLauncher::activeCallCount(int count)
{
    m_activeCalls = count;
}

/*!
  \internal
  */
void PhoneLauncher::registrationChanged()
{
    QList<QAbstractCallPolicyManager *> managers;
    QString pix;
    QString msg;

    managers = qtopiaTasks<QAbstractCallPolicyManager>();
    foreach (QAbstractCallPolicyManager *manager, managers) {
        if (pix.isEmpty())
            pix = manager->registrationIcon();
        QString newMsg = manager->registrationMessage();
        if (!newMsg.isEmpty()) {
            if (!msg.isEmpty())
                msg += "<br>";
            msg.append(newMsg);
        }
    }

    if (m_registrationMessageId) {
        m_homeScreen->clearInformation(m_registrationMessageId);
        m_registrationMessageId = 0;
    }

    if (!msg.isEmpty())
        m_registrationMessageId = m_homeScreen->showInformation(pix, msg);
}

#endif

/*!
  \internal
  */
void PhoneLauncher::timerEvent(QTimerEvent * tev)
{
    if(tev && tev->timerId() == m_updateTid) {
        update();
        killTimer(m_updateTid);
        m_updateTid = 0;
        return;
    }

    if ( tev && m_multitaskingMultipressTimer.timerId() == tev->timerId() ) {
        m_multitaskingMultipressTimer.stop();
        return;
    }
}

#ifdef QTOPIA_PHONEUI
/*!
  \internal
  Shows the Missed Calls dialog if there are any missed calls.
*/
void PhoneLauncher::showAlertDialogs()
{
    if (isVisible()) {
        if(DialerControl::instance()->missedCallCount() &&
           DialerControl::instance()->missedCallCount() != m_alertedMissed) {
            m_alertedMissed = DialerControl::instance()->missedCallCount();
            if (!m_missedMessageBox) {
                QString missedMsg = tr("Do you wish to view missed calls?");
                m_missedMessageBox = QAbstractMessageBox::messageBox(m_homeScreen, tr("Missed Call"),
                        "<qt>"+missedMsg+"</qt>", QAbstractMessageBox::Information,
                        QAbstractMessageBox::Yes, QAbstractMessageBox::No);
                connect(m_missedMessageBox, SIGNAL(finished(int)), this, SLOT(messageBoxDone(int)));
            }
            m_missedMessageBox->setTimeout(NotificationVisualTimeout, QAbstractMessageBox::No);
            QtopiaApplication::showDialog(m_missedMessageBox);
        }
    }
}
#endif

/*!
  \internal
  */
void PhoneLauncher::keyStateChanged(bool locked)
{
#ifdef QTOPIA_PHONEUI
    if(!locked)
        showAlertDialogs();
#else
    Q_UNUSED(locked);
#endif
}

/*!
  \internal
  */
void PhoneLauncher::updateBackground()
{
    m_homeScreen->updateBackground();
}

/*!
  \internal
  */
void PhoneLauncher::polishWindows()
{
    ThemeBackground::polishWindows(ThemeBackground::PrimaryScreen);
}

/*!
  \internal
  */
void PhoneLauncher::paintEvent(QPaintEvent *)
{
    QPixmap pm(":image/qpelogo");
    QPainter p(this);
    p.drawPixmap((width()-pm.width())/2,( height()-pm.height())/2, pm);
}

/*!
  \internal
  */
void PhoneLauncher::updateLauncherIconSize()
{
}

/*!
  \internal
  */
void PhoneLauncher::createContext()
{
    Q_ASSERT(!m_context);

    m_context = new ContextLabel(0, Qt::FramelessWindowHint |
                                    Qt::Tool |
                                    Qt::WindowStaysOnTopHint );

    m_context->move(QApplication::desktop()->screenGeometry().topLeft()+QPoint(0,50)); // move to the correct screen
    WindowManagement::protectWindow(m_context);
    m_context->setAttribute(Qt::WA_GroupLeader);
    ThemeControl::instance()->registerThemedView(m_context, "Context");

    Atom atom = XInternAtom(QX11Info::display(), "_QTOPIA_SOFT_MENU", False);
    unsigned long flag = 1;
    XChangeProperty(QX11Info::display(), m_context->winId(), atom, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&flag, 1);
}

/*!
  \internal
  */
ContextLabel *PhoneLauncher::context()
{
    Q_ASSERT(m_context);
    return m_context;
}

#ifdef QTOPIA_PHONEUI
/*!
  \internal
  Displays the dialer, preloaded with \a digits.  If \a speedDial
  is true, the digits will be considered for use in speed dial (press and
  hold).
*/
void PhoneLauncher::showDialer(const QString &digits, bool speedDial)
{
    if (!dialer())
        return;

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
  \internal
  Displays the call history window.  If \a missed is true, the missed
  calls tab will be on top.  If \a hint is not empty, it is used to
  highlight a matching call.
*/
void PhoneLauncher::showCallHistory(bool missed, const QString &hint)
{
#if defined(QTOPIA_TELEPHONY)
    if ( !callHistory() )
        initializeCallHistory();

    callHistory()->reset();
    if (missed || DialerControl::instance()->missedCallCount() > 0)
        callHistory()->showMissedCalls();
    if( hint.length() )
        callHistory()->setFilter( hint );

    callHistory()->refresh();

    if( !callHistory()->isHidden() )
    {
        callHistory()->raise();
    }
    else
    {
        callHistory()->showMaximized();
    }
#else
    Q_UNUSED(missed);
    Q_UNUSED(hint);
#endif
}

/*!
  \internal
  Initialize call history window.
  This delayed initilization will shorten the first launch time.
*/
void PhoneLauncher::initializeCallHistory()
{
#if defined(QTOPIA_TELEPHONY)
    if ( !m_callHistory ) {
        m_callHistory = new CallHistory(DialerControl::instance()->callList(), 0);
        QtopiaApplication::prepareMainWidget(m_callHistory);
        connect(callHistory(), SIGNAL(viewedMissedCalls()),
                this, SLOT(resetMissedCalls()) );
        connect(callHistory(), SIGNAL(viewedMissedCalls()),
                DialerControl::instance(), SLOT(resetMissedCalls()) );
        connect(callHistory(),
                SIGNAL(requestedDial(QString,QUniqueId)),
                this,
                SLOT(requestDial(QString,QUniqueId)));
    }
#endif
}
#endif

#ifdef QTOPIA_CELL

/*!
  \internal
  */
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
/*!
  \internal
  */
void PhoneLauncher::ussdMessage(const QString &text)
{
    QString title = tr("Service request");
    QString displayText = "<qt>" + text + "</qt>";
    if (!m_serviceMessageBox) {
        m_serviceMessageBox = QAbstractMessageBox::messageBox(0, title, displayText,
                                       QAbstractMessageBox::Information);
    } else {
        m_serviceMessageBox->setWindowTitle(title);
        m_serviceMessageBox->setText(displayText);
    }
    QtopiaApplication::showDialog(m_serviceMessageBox);
}
#endif

#ifdef QTOPIA_CELL

/*!
  \internal
  */
void PhoneLauncher::cellBroadcast(CellBroadcastControl::Type type,
                                  const QString &chan, const QString &text)
{
    if(type != CellBroadcastControl::Popup)
        return;

    // remove the previous message box if there is any
    // user will see the latest one only
    delete m_CBSMessageBox;
    m_CBSMessageBox = QAbstractMessageBox::messageBox(m_homeScreen, chan,
                                   text, QAbstractMessageBox::Information,
                                   QAbstractMessageBox::No);
    QtopiaApplication::showDialog(m_CBSMessageBox);
}

#endif // QTOPIA_CELL


#ifdef QTOPIA_PHONEUI
/*!
  \internal
  */
void PhoneLauncher::stateChanged()
{
    callScreen()->stateChanged(); // We must know that this is updated before we continue

    if (DialerControl::instance()->hasIncomingCall()) {
        if ((m_callTypeMessageBox && m_callTypeMessageBox->isVisible())
#ifdef QTOPIA_VOIP
            || (m_voipNoPresenceMessageBox && m_voipNoPresenceMessageBox->isVisible())
#endif
            ) {
            // assume the request to call is cancelled
            m_queuedCall = QString();
            m_queuedCallType = QString();
            m_queuedCallContact = QUniqueId();
        }
        showCallScreen();
    }

    if (m_queuedIncoming && !DialerControl::instance()->hasIncomingCall()) {
        // incoming call has gone away
        m_queuedIncoming = false;
        delete m_incomingMessageBox;
        m_incomingMessageBox = 0;
    }

    if (m_queuedIncoming || !m_queuedCall.isEmpty()) {
        bool haveFg = DialerControl::instance()->hasActiveCalls();
        if (haveFg && !DialerControl::instance()->hasCallsOnHold()) {
            DialerControl::instance()->hold();
        } else if (!haveFg) {
            showCallScreen();
            if (m_queuedIncoming) {
                m_queuedIncoming = false;
                DialerControl::instance()->accept();
            } else {
                QString n(m_queuedCall);
                m_queuedCall = QString();
                m_queuedCallContact = QUniqueId();
                DialerControl::instance()->dial(n,DialerControl::instance()->callerIdNeeded(n), m_queuedCallType, m_queuedCallContact);
            }
        }
    }
}

/*!
  \internal
  */
void PhoneLauncher::acceptIncoming()
{
    if (DialerControl::instance()->hasIncomingCall()) {
        showCallScreen();
        if(DialerControl::instance()->hasActiveCalls() &&
           DialerControl::instance()->hasCallsOnHold()) {
            if (!m_incomingMessageBox) {
                m_incomingMessageBox = QAbstractMessageBox::messageBox(
                    callScreen(),
                    tr("End current call?"),
                    tr("<qt>Do you wish to end the current call before answering the incoming call?</qt>"),
                    QAbstractMessageBox::Warning, QAbstractMessageBox::Yes, QAbstractMessageBox::No);
                connect(m_incomingMessageBox, SIGNAL(finished(int)), this, SLOT(messageBoxDone(int)));
            }
            QtopiaApplication::showDialog(m_incomingMessageBox);
            m_queuedIncoming = true;
        } else {
            DialerControl::instance()->accept();
        }
    }
}

/*!
  \internal
  Accepts an incoming call.  Has no affect if an incoming call is not
  currently available.
*/
void PhoneLauncher::messageBoxDone(int r)
{
    QAbstractMessageBox *box = (QAbstractMessageBox*)sender();
    if (box == m_missedMessageBox && r == QAbstractMessageBox::Yes) {
        showCallHistory(true);
    } else if (box == m_dialingMessageBox) {
        if (r == QAbstractMessageBox::Yes)
            DialerControl::instance()->endCall();
        else
        {
            m_queuedCall = QString();
            m_queuedCallContact = QUniqueId();
        }
    } else if (box == m_incomingMessageBox) {
        if (r == QAbstractMessageBox::Yes) {
            DialerControl::instance()->endCall();
        } else {
            m_queuedIncoming = false;
            if (DialerControl::instance()->hasIncomingCall())
                DialerControl::instance()->incomingCall().hangup();
        }
    } else if (box == m_callTypeMessageBox) {
        switch(r) {
        case 0:
            m_queuedCallType = "Voice";     // No tr
            dialNumber(m_queuedCall, m_queuedCallContact, m_queuedCallType);
            break;
        case 1:
            m_queuedCallType = "VoIP";      // No tr
#ifdef QTOPIA_VOIP
            if ( VoIPManager::instance()->isAvailable( m_queuedCall ) ) {
                dialNumber(m_queuedCall, m_queuedCallContact, m_queuedCallType);
            } else {
                if (!m_voipNoPresenceMessageBox) {
                    m_voipNoPresenceMessageBox = QAbstractMessageBox::messageBox(callScreen(), tr("Unavailable"),
                        tr("<qt>The selected contact appears to be unavailable. Do you still wish to make a call?</qt>"),
                        QAbstractMessageBox::Warning, QAbstractMessageBox::Yes, QAbstractMessageBox::No);
                    connect(m_voipNoPresenceMessageBox, SIGNAL(finished(int)), this, SLOT(messageBoxDone(int)));
                }
                QtopiaApplication::showDialog(m_voipNoPresenceMessageBox);
            }
#endif
            break;
        default:
            break;
        }
    }
#ifdef QTOPIA_VOIP
    else if (box == m_voipNoPresenceMessageBox) {
        if (r == QAbstractMessageBox::Yes) {
            dialNumber(m_queuedCall, m_queuedCallContact, m_queuedCallType);
        } else {
            m_queuedCall = QString();
            m_queuedCallType = QString();
            m_queuedCallContact = QUniqueId();
        }
    }
#endif
}
#endif

/*!
  \internal
  This function is now a public slot.
 */
void PhoneLauncher::showWarning(const QString &title, const QString &text)
{
    if (!m_warningMessageBox)
        m_warningMessageBox =
	    QAbstractMessageBox::messageBox(0,title,text,
					    QAbstractMessageBox::Warning);
    m_warningMessageBox->setWindowTitle(title);
    m_warningMessageBox->setText(text);
    m_warningMessageBox->setTimeout(WarningTimeout,QAbstractMessageBox::NoButton);
    QtopiaApplication::showDialog(m_warningMessageBox);
}

#ifdef QTOPIA_PHONEUI
/*!
  Dials number \a n.  The current call state is first checked to confirm
  that it is possible to make a call.  If it is not possible to make a
  call, the user will be notified, otherwise the call screen will be shown
  and the call initiated.
  \internal
*/
void PhoneLauncher::dialNumber(const QString &n, const QUniqueId &c, const QString &callType)
{
    if (callType.isEmpty() || n.isEmpty())
        return;

    // Save service request history.
    /*
    QtopiaServiceRequest req("Dialer", "dial(QString,QUniqueId)");
    req << n << c;
    QString label;
    QString icon(":icon/phone/phone"); // no tr
    if (!c.isNull()) {
        QContactModel *m = ServerContactModel::instance();
        QContact contact = m->contact(c);
        label = tr("Call %1").arg(contact.label());
        QMap<QContact::PhoneType, QString> numbers = contact.phoneNumbers();
        QMap<QContact::PhoneType, QString>::iterator it;
        for (it = numbers.begin(); it != numbers.end(); ++it) {
            if (*it == n) {
                icon = contact.phoneIconResource(it.key());
                break;
            }
        }
    } else {
        label = tr("Call %1").arg(n);
    }
    QtopiaServiceHistoryModel::insert(req, label, icon);
    */

    showCallScreen();

    if (DialerControl::instance()->hasActiveCalls() && DialerControl::instance()->hasCallsOnHold()) {
        if (!m_dialingMessageBox) {
            m_dialingMessageBox = QAbstractMessageBox::messageBox(
                    callScreen(),
                    tr("End current call?"),
                    tr("<qt>Do you wish to end the current call before begining the new call?</qt>"),
                    QAbstractMessageBox::Warning, QAbstractMessageBox::Yes, QAbstractMessageBox::No);
             connect(m_dialingMessageBox, SIGNAL(finished(int)), this, SLOT(messageBoxDone(int)));
        }
        QtopiaApplication::showDialog(m_dialingMessageBox);
    } else if (DialerControl::instance()->hasActiveCalls()) {
        DialerControl::instance()->hold();
    } else {
        DialerControl::instance()->dial( n, DialerControl::instance()->callerIdNeeded(n), callType, c );
    }
}

class CallTypeSelector : public PhoneMessageBox
{
    Q_OBJECT
public:
    CallTypeSelector( const QList<QAbstractCallPolicyManager *>& managers,
                      QWidget *parent = 0 );
    ~CallTypeSelector();

    QAbstractCallPolicyManager *selectedPolicyManager() const;

private slots:
    void itemActivated();

private:
    QListWidget *list;
    QList<QAbstractCallPolicyManager *> managers;
};

CallTypeSelector::CallTypeSelector
    ( const QList<QAbstractCallPolicyManager *>& managers, QWidget *parent )
    : PhoneMessageBox( parent )
{
    this->managers = managers;

    setIcon(QAbstractMessageBox::Warning);
    setText(tr("Which type of call do you wish to make?"));

    list = new QListWidget(this);
    list->setSortingEnabled(true);
    list->setItemDelegate(new QtopiaItemDelegate);
    list->setFrameStyle(QFrame::NoFrame);
    addContents(list);

    foreach ( QAbstractCallPolicyManager *manager, managers ) {
        QListWidgetItem *item = new QListWidgetItem(list);
        item->setText(manager->trCallType());
        item->setIcon(QIcon(":icon/" + manager->callTypeIcon()));
        item->setData(Qt::UserRole, manager->callType());
        list->addItem(item);
    }
    list->setCurrentItem(list->item(0));

    connect(list, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(itemActivated()));
}

CallTypeSelector::~CallTypeSelector()
{
}

QAbstractCallPolicyManager *CallTypeSelector::selectedPolicyManager() const
{
    QListWidgetItem *current = list->currentItem();
    if ( !current )
        return 0;
    QString callType = current->data(Qt::UserRole).toString();
    foreach ( QAbstractCallPolicyManager *manager, managers ) {
        if ( manager->callType() == callType )
            return manager;
    }
    return 0;
}

void CallTypeSelector::itemActivated()
{
    done(QAbstractMessageBox::Yes);
}

/*!
  \internal
  */
void PhoneLauncher::requestDial(const QString &n, const QUniqueId &c)
{
    if (n.isEmpty())
        return;

    int numberAtSymbol = n.count('@');
    if (numberAtSymbol > 1) {
        showWarning(tr("Incorrect Number format"),
                tr("<qt>Unable to make a phone call.</qt>"));
        return;
    }

    // Ask all of the call policy managers what they want to do
    // with this phone number.
    QAbstractCallPolicyManager::CallHandling handling;
    QList<QAbstractCallPolicyManager *> managers;
    QList<QAbstractCallPolicyManager *> candidates;
    QAbstractCallPolicyManager *chosenManager = 0;
    managers = qtopiaTasks<QAbstractCallPolicyManager>();
    foreach (QAbstractCallPolicyManager *manager, managers) {
        handling = manager->handling(n);
        if ( handling == QAbstractCallPolicyManager::MustHandle ) {
            chosenManager = manager;
            break;
        } else if ( handling == QAbstractCallPolicyManager::NeverHandle ) {
            chosenManager = 0;
            candidates.clear();
            break;
        } else if ( handling == QAbstractCallPolicyManager::CanHandle ) {
            candidates.append( manager );
        }
    }

    // Bail out if nothing can dial this number at this time.
    if (!chosenManager && candidates.isEmpty()) {
        showWarning(tr("No GSM/VoIP Network"),
                tr("<qt>No phone call is possible.</qt>"));
        return;
    }

    m_queuedCall = n;
    m_queuedCallType = QString();
    m_queuedCallContact = c;

#ifdef QTOPIA_CELL
    if(m_cellModem->planeModeEnabled()) {
        // At the moment, plane mode is the only way to turn
        // the GSM module off.
        showWarning(tr("Airplane Mode"),
            tr("<qt>Phone calls cannot be made in Airplane Mode.</qt>"));
        return;
    }
#endif

    // Determine which call policy manager to use.
    if (!chosenManager) {
        if (candidates.size() == 1) {
            // Only one call policy manager is active, so use that.
            chosenManager = candidates[0];
        } else {
            // Clear queued call information while the dialog is up
            // to prevent the call from being automatically dialed
            // if we get to PhoneLauncher::stateChanged() while
            // the dialog is on-screen.
            m_queuedCall = QString();
            m_queuedCallType = QString();
            m_queuedCallContact = QUniqueId();

            // More than one is active, so we have to ask the user.
            CallTypeSelector selector( candidates );
            if (QtopiaApplication::execDialog(&selector) != QAbstractMessageBox::Yes) {
                return;
            }
            chosenManager = selector.selectedPolicyManager();
            if ( !chosenManager ) {
                // Shouldn't happen, but recover gracefully anyway.
                return;
            }

            // Re-instate the queued call information.
            m_queuedCall = n;
            m_queuedCallType = QString();
            m_queuedCallContact = c;
        }
    }

    // Get the chosen call type.
    m_queuedCallType = chosenManager->callType();

#ifdef QTOPIA_VOIP
    // Ask the user to confirm if the called party is not present.
    if (!chosenManager->isAvailable(m_queuedCall)) {
        if (!m_voipNoPresenceMessageBox) {
            m_voipNoPresenceMessageBox = QAbstractMessageBox::messageBox(callScreen(), tr("Unavailable"),
                tr("<qt>The selected contact appears to be unavailable. Do you still wish to make a call?</qt>"),
                QAbstractMessageBox::Warning, QAbstractMessageBox::Yes, QAbstractMessageBox::No);
            connect(m_voipNoPresenceMessageBox, SIGNAL(finished(int)), this, SLOT(messageBoxDone(int)));
        }
        QtopiaApplication::showDialog(m_voipNoPresenceMessageBox);
        return;
    }
#endif

    // Dial the specified number.
    dialNumber(m_queuedCall, m_queuedCallContact, m_queuedCallType);
}

/*!
  \internal
  */
void PhoneLauncher::serviceNumber(QServiceNumbers::NumberId id, const QString& number)
{
    if ( id == QServiceNumbers::VoiceMail ) {
        // The "m_waitingVoiceMailNumber" flag is used to prevent
        // false positives when the "phonesettings" program
        // queries for the voice mail number.
        if ( m_waitingVoiceMailNumber ) {
            m_waitingVoiceMailNumber = false;
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
  \internal
    Dial the voice mail number in the SIM.
*/
void PhoneLauncher::dialVoiceMail()
{
    m_waitingVoiceMailNumber = true;
    if ( m_serviceNumbers->available() )
        m_serviceNumbers->requestServiceNumber( QServiceNumbers::VoiceMail );
    else
        serviceNumber( QServiceNumbers::VoiceMail, QString() );
}

/*!
  \internal
  */
void PhoneLauncher::resetMissedCalls()
{
    m_alertedMissed = 0;
}

/*!
  \internal
  */
QAbstractHomeScreen *PhoneLauncher::homeScreen() const
{
    return m_homeScreen;
}

/*!
  \internal
  */
QAbstractDialerScreen *PhoneLauncher::dialer(bool create) const
{
    if(create && !m_dialer) {
        m_dialer = qtopiaWidget<QAbstractDialerScreen>(0);
        if (!m_dialer) {
            qLog(UI) << "Unable to create the Dialer Screen";
            return 0;
        }

        QtopiaApplication::prepareMainWidget(m_dialer);
        m_dialer->move(QApplication::desktop()->screenGeometry().topLeft());

        connect(m_dialer,
                SIGNAL(requestDial(QString,QUniqueId)),
                this,
                SLOT(requestDial(QString,QUniqueId)));

    #ifdef QTOPIA_CELL
        m_gsmKeyActions->setDialer(m_dialer);
    #endif
    }

    return m_dialer;
}

/*!
  \internal
  */
CallScreen *PhoneLauncher::callScreen(bool create) const
{
    if(create && !m_callScreen) {
        m_callScreen = new CallScreen(DialerControl::instance(), 0);
        m_callScreen->move(QApplication::desktop()->screenGeometry().topLeft());
        QtopiaApplication::prepareMainWidget(m_callScreen);

        RingControl *rc = qtopiaTask<RingControl>();
        if(rc)
            QObject::connect(m_callScreen, SIGNAL(muteRing()),
                             rc, SLOT(muteRing()));
        QObject::connect(m_callScreen, SIGNAL(listEmpty()),
                         m_callScreen, SLOT(hide()));
        QObject::connect(m_callScreen, SIGNAL(acceptIncoming()),
                         this, SLOT(acceptIncoming()));
        ThemeControl::instance()->registerThemedView(m_callScreen, "CallScreen");

#ifdef QTOPIA_CELL
        QObject::connect( m_callScreen, SIGNAL(filterKeys(QString,bool&)),
                          m_gsmKeyActions, SLOT(filterKeys(QString,bool&)) );
        QObject::connect( m_callScreen, SIGNAL(filterSelect(QString,bool&)),
                          m_gsmKeyActions, SLOT(filterSelect(QString,bool&)) );
        QObject::connect( m_callScreen, SIGNAL(testKeys(QString,bool&)),
                          m_gsmKeyActions, SLOT(testKeys(QString,bool&)) );
#endif
    }
    return m_callScreen;

}

#endif

void PhoneLauncher::newMessagesChanged()
{
    m_homeScreen->setNewMessages(m_newMessages.value().toInt());
}


QTOPIA_REPLACE_WIDGET(QAbstractServerInterface, PhoneLauncher);

#include "phonelauncher_illume.moc"
