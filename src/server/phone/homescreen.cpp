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

#include <QAction>
#include <QDebug>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QList>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QSettings>
#include <QTimer>

#include <qtopiaapplication.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>

#include <qsoftmenubar.h>
#include <qdevicebuttonmanager.h>
#include <qtopia/private/themedviewinterface_p.h>

#include "phonelauncher.h"
#ifdef QTOPIA_CELL
#include "cellmodemmanager.h"
#endif
#ifdef QTOPIA_PHONEUI
#include "dialercontrol.h"
#endif
#include "homescreen.h"
#include "taskmanagerservice.h"
#include "pressholdgate.h"
#include <qdrmcontent.h>
#if defined(QTOPIA_BLUETOOTH) || defined(QTOPIA_INFRARED)
#include "obexservicemanager.h"
#include "receivewindow.h"
#endif

#include "homescreenwidgets.h"

// ============================================================================
//
// ThemeBackground
//
// ============================================================================

class ThemeBackground : public ThemedItemPlugin
{
    Q_OBJECT
public:
    ThemeBackground();
    virtual ~ThemeBackground() {}

    void resize(int w, int h);
    void paint(QPainter *p, const QRect &r);

    enum DisplayMode { Center, Scale, Tile };

#ifdef QTOPIA4_TODO
    QRESULT queryInterface(const QUuid&, QUnknownInterface**);
    Q_REFCOUNT
#endif

private slots:
    void rightsExpired( const QDrmContent &content );

private:
    QDrmContent imgContent;
    QString imgName;
    QPixmap bg;
    int width;
    int height;
    DisplayMode dpMode;
    ulong ref;

};

bool HomeScreen::keyLocked() const
{
    return keyLock->locked();
}

void HomeScreen::setKeyLocked(bool lock) const
{
    if(lock)
        keyLock->lock();
    else
        keyLock->unlock();
}

ThemeBackground::ThemeBackground()
    : ThemedItemPlugin(), width(0), height(0), dpMode(Center), ref(0)
{
    QSettings cfg("Trolltech","qpe");
    cfg.beginGroup("Appearance");
    imgName = cfg.value("BackgroundImage").toString();
    dpMode = (DisplayMode)cfg.value("BackgroundMode").toInt();
    if( !imgName.isEmpty() )
    {
        if( imgName[ 0 ] != '/' )
            imgName = QString(":image/wallpaper/"+imgName);
        else
        {
            imgContent.setPermission( QDrmRights::Display );
            imgContent.setLicenseOptions( QDrmContent::Handover | QDrmContent::Reactivate );

            connect( &imgContent, SIGNAL(rightsExpired(const QDrmContent&)),
                      this,       SLOT  (rightsExpired(const QDrmContent&)) );

            if( !imgContent.requestLicense( QContent( imgName, false ) ) )
                return;

            imgContent.renderStarted();
        }
    }
    qLog(UI) << "Loading background image:" << imgName;
    bg.load(imgName);
}

void ThemeBackground::resize(int w, int h)
{
    width = w;
    height = h;
    if (!imgName.isEmpty()) {
        if (dpMode == Center || dpMode == Tile) {
            if (bg.size().width() > width || bg.size().height() > height )
                bg = bg.scaled(QSize(width, height), Qt::KeepAspectRatio);
        } else if (dpMode == Scale) {
            bg = bg.scaled(QSize(width, height), Qt::KeepAspectRatioByExpanding);
        }
    }
    qLog(UI) << "Background image resized:" << bg.size().width() << "x" << bg.size().height();
    qLog(UI) << "Background image disply mode:"
        << ((dpMode == Center) ? "Center" : (dpMode == Scale) ? "Maximize" : "Tile");
}

void ThemeBackground::paint(QPainter *p, const QRect &r)
{
    if (!bg.isNull()) {
        if (dpMode == Tile) {
            int dx = r.x() % bg.width();
            int dy = r.y() % bg.height();
            p->drawTiledPixmap(r.x()-dx, r.y()-dy,
                            r.width()+dx, r.height()+dy, bg);
        } else if (dpMode == Center || dpMode == Scale) {
            QPoint off((width-bg.width())/2, (height-bg.height())/2);
            QRect cr = r;
            cr.translate(-off.x(), -off.y());
            cr &= QRect(0, 0, bg.width(), bg.height());
            p->drawPixmap(cr.topLeft()+off, bg, cr);
        }
    }
}

#ifdef QTOPIA4_TODO
QRESULT ThemeBackground::queryInterface(const QUuid &uuid, QUnknownInterface **iface)
{
    *iface = 0;
    if (uuid == IID_QUnknown)
        *iface = this;
    else if (uuid == IID_ThemedItem)
        *iface = this;
    else
        return QS_false;

    (*iface)->addRef();
    return QS_OK;
}
#endif

void ThemeBackground::rightsExpired( const QDrmContent &content )
{
    Q_UNUSED( content );

    QSettings config("Trolltech","qpe");
    config.beginGroup( "Appearance" );

    config.setValue("BackgroundImage", QString() );

    config.sync(); // need to flush the config info first

    QtopiaChannel::send("QPE/System", "applyStyle()");

#ifdef QTOPIA_PHONE
    QtopiaChannel::send("QPE/System", "applyStyleSplash()");
#endif

}

//===========================================================================

HomeScreen::HomeScreen(QWidget *parent, Qt::WFlags f)
    : PhoneThemedView(parent, f), keyLock(0),
#ifdef QTOPIA_CELL
      simLock(0),
#endif
      lockTimer(0), lockMsgId(0), infoMsgId(0), missedCalls(0), bgIface(0)
{
    keyLock = new BasicKeyLock(this);

    QObject::connect(keyLock, SIGNAL(stateChanged(BasicKeyLock::State,QString)),
                     this, SLOT(showLockInformation()));
    QObject::connect(keyLock, SIGNAL(keyLockDetected()),
                     keyLock, SLOT(lock()));

#ifdef QTOPIA_CELL
    simLock = new BasicSimPinLock(this);

    QObject::connect(simLock,
                     SIGNAL(stateChanged(BasicSimPinLock::State,QString)),
                     this, SLOT(showLockInformation()));
    QObject::connect(keyLock, SIGNAL(dialEmergency(QString)),
                     this, SIGNAL(callEmergency(QString)));
    QObject::connect(simLock, SIGNAL(dialEmergency(QString)),
                     this, SIGNAL(callEmergency(QString)));
#endif

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_InputMethodEnabled);

    setContextBarLocked(false);
    setFocusPolicy(Qt::StrongFocus);

    QMenu *contextMenu = QSoftMenuBar::menuFor(this);
    m_contextMenu = contextMenu;
    contextMenu->installEventFilter(this);

    actionLock = new QAction(QIcon(":icon/padlock"), tr("Key Lock"), this);
    if (!Qtopia::mousePreferred()) {
        connect(actionLock, SIGNAL(triggered()), keyLock, SLOT(lock()));
        contextMenu->addAction(actionLock);
    }

    QAction *actionProfile = new QAction(QIcon( ":icon/Note" ), tr("Profile..."), this);
    connect( actionProfile, SIGNAL(triggered()), this, SLOT(showProfileSelector()) );
    contextMenu->addAction(actionProfile);

#ifdef QTOPIA_PHONEUI
    connect(DialerControl::instance(), SIGNAL(stateChanged()),
            this, SLOT(phoneStateChanged()));

    actionCalls = new QAction(QIcon(":icon/phone/missedcall"), tr("Missed Calls..."), this);
    connect(actionCalls, SIGNAL(triggered()), this, SLOT(viewMissedCalls()));
    contextMenu->addAction(actionCalls);
    actionCalls->setEnabled(false);
    actionCalls->setVisible(false);

    actionMessages = new QAction(QIcon(":icon/phone/sms"), tr("New Messages..."), this);
    connect(actionMessages, SIGNAL(triggered()), this, SLOT(viewNewMessages()));
    contextMenu->addAction(actionMessages);
    actionMessages->setEnabled(false);
    actionMessages->setVisible(false);
#endif

#ifdef QTOPIA_CELL
    CellBroadcastControl *cbc = CellBroadcastControl::instance();
    connect(cbc,
            SIGNAL(broadcast(CellBroadcastControl::Type,QString,QString)),
            this,
            SLOT(cellBroadcast(CellBroadcastControl::Type,QString,QString)));
#endif

    QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::NoLabel);

    if (Qtopia::mousePreferred()) {
        QtopiaApplication::setInputMethodHint(this, "phoneonly");
        setFocus();
    }
    speeddialTimer = new QTimer(this);
    speeddialTimer->setSingleShot(true);
    speeddial_preedit = 0;
    connect(speeddialTimer, SIGNAL(timeout()), this, SLOT(activateSpeedDial()));

    ph = new PressHoldGate("HomeScreen",this);
    connect(ph,SIGNAL(activate(int,bool)),this,SLOT(specialButton(int,bool)));

    QObject::connect(this, SIGNAL(itemClicked(ThemeItem*)),
                     this, SLOT(themeItemClicked(ThemeItem*)));

#if defined(QTOPIA_BLUETOOTH) || defined(QTOPIA_INFRARED)
    ObexServiceManager *obexmgr = new ObexServiceManager(this);
    ReceiveWindow *recvWindow = new ReceiveWindow(this);

    connect(obexmgr, SIGNAL(receiveInitiated(int, const QString &, const QString &)),
            recvWindow, SLOT(receiveInitiated(int, const QString &, const QString &)));
    connect(obexmgr, SIGNAL(sendInitiated(int, const QString &, const QString &)),
            recvWindow, SLOT(sendInitiated(int, const QString &, const QString &)));
    connect(obexmgr, SIGNAL(progress(int, qint64, qint64)),
            recvWindow, SLOT(progress(int, qint64, qint64)));
    connect(obexmgr, SIGNAL(completed(int, bool)),
            recvWindow, SLOT(completed(int, bool)));
#endif

    installEventFilter(this);
    showLockInformation();
}

void HomeScreen::activateSpeedDial()
{
    if ( speeddialdown )
        emit speedDial(speeddial);
    if (speeddial_preedit)
        speeddial_activated_preedit = speeddial.right(speeddial_preedit);
    speeddial_preedit = 0;
    speeddial.clear();
}

HomeScreen::~HomeScreen()
{
}

void HomeScreen::themeLoaded(const QString& /*theme*/)
{
    applyBackgroundImage();

    ThemeTextItem *textItem = (ThemeTextItem *)findItem("infobox", Text);
    if (textItem)
        textItem->setTextFormat(Qt::RichText);
}

void HomeScreen::applyBackgroundImage()
{
    ThemePluginItem *ip = (ThemePluginItem *)findItem("bgplugin", Plugin);
    if (ip) {
        QSettings cfg("Trolltech","Launcher");
        cfg.beginGroup("HomeScreen");
        QString pname = cfg.value("Plugin", "Background").toString();
        if (pname == "Background") {
            bgIface = new ThemeBackground;
            ip->setBuiltin(bgIface);
        } else {
            ip->setPlugin(pname);
        }
    }
}

#ifdef QTOPIA_PHONEUI
void HomeScreen::setMissedCalls(int m)
{
    actionCalls->setEnabled(m);
    actionCalls->setVisible(m);
}

void HomeScreen::setNewMessages(int m)
{
    actionMessages->setEnabled(m);
    actionMessages->setVisible(m);
}
#endif

void HomeScreen::setContextBarLocked(bool locked)
{
    if (locked)
        QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
    else
        QSoftMenuBar::setLabel(this, Qt::Key_Select, "qpe/menu", tr("Menu"));

    if ( locked ) {
        QSoftMenuBar::setLabel(this, BasicKeyLock::lockKey(), "unlock", tr("Unlock"));
    } else {
        if (BasicKeyLock::lockKey() == QSoftMenuBar::menuKey()) {
            QSoftMenuBar::setLabel(this, BasicKeyLock::lockKey(), QSoftMenuBar::Options);
        } else {
            QSoftMenuBar::clearLabel(this, BasicKeyLock::lockKey());
        }
    }
}

void HomeScreen::showProfileSelector()
{
    QtopiaServiceRequest e( "Profiles", "showProfiles()" );
    e.send();
}

void HomeScreen::themeItemClicked(ThemeItem *item)
{
    if( !item->isInteractive() )
        return;

    // TODO make interactivity with HSWIDGETS generic
    QString in = item->itemName();
    if ( in == "LauncherHSWidget" ) {
        ThemeWidgetItem* wItem = static_cast<ThemeWidgetItem*>( item );
        if ( wItem != 0 ) {
            LauncherHSWidget* launcher
                = qobject_cast<LauncherHSWidget*>( wItem->widget() );
            if ( launcher != 0 )
                launcher->launch();
        }
    } else if ( in == "WorldmapHSWidget" ) {
        ThemeWidgetItem* wItem = static_cast<ThemeWidgetItem*>( item );
        if ( wItem != 0 ) {
            WorldmapHSWidget* worldmap
                = qobject_cast<WorldmapHSWidget*>( wItem->widget() );
            if ( worldmap != 0 )
                worldmap->showCity();
        }
    }
}

void HomeScreen::showLockInformation()
{
    // We "lock" navigation throughout the phone if either the key or sim lock
    // is on unless a call is in progress!
#ifdef QTOPIA_PHONEUI
    bool calls = DialerControl::instance()->allCalls().count() != 0;
#else
    bool calls = false;
#endif
#ifdef QTOPIA_CELL
    bool lock = !calls && (keyLock->locked() || !simLock->open());
#else
    bool lock = !calls && (keyLock->locked());
#endif

    QString text;
    QString pix("padlock");

    if(lock) {
        m_contextMenu->hide();
        setContextBarLocked(true);
            } else {
        setContextBarLocked(false);
                    }

    if(!lock) {
        // No lock
        if (Qtopia::mousePreferred())
            QtopiaApplication::setInputMethodHint(this, "phoneonly");
    } else if(keyLock->emergency()
#ifdef QTOPIA_CELL
              || simLock->emergency()
#endif
              ) {
        // Emergency dial!
        bool partial = false;
        QString number;

#ifdef QTOPIA_CELL
        if(simLock->state() == BasicSimPinLock::EmergencyNumber) {
            partial = false;
            number = simLock->number();
        } else
#endif
        if(keyLock->state() == BasicKeyLock::EmergencyNumber) {
            partial = false;
            number = keyLock->emergencyNumber();
        } else
#ifdef QTOPIA_CELL
        if(simLock->state() == BasicSimPinLock::PartialEmergencyNumber) {
            partial = true;
            number = simLock->number();
        } else
#endif
        if(keyLock->state() == BasicKeyLock::PartialEmergencyNumber) {
            partial = true;
            number = keyLock->emergencyNumber();
                }

        if(partial) {
            text = QString("<b>")+number+QString("</b>");
        } else {
            text = tr("Emergency number <b>%1</b><br>Press "
                      "<font color=#008800>Call</font> to dial.").arg(number);
            pix = "emergency";
                    }
    } else if(keyLock->locked()) {
        // Key lock
        switch(keyLock->state()) {
            case BasicKeyLock::KeyLocked:
                    break;
            case BasicKeyLock::KeyLockIncorrect:
                    {
                    text = tr("Press %1 then * to unlock phone");
                    int lkey = BasicKeyLock::lockKey();
                    if (lkey == Qt::Key_Context1)
                        text = text.arg("Unlock");
                    else
                        text = text.arg(QString(QKeySequence(lkey)));
                    }
                    break;
            case BasicKeyLock::KeyLockToComplete:
                text = tr("Now press * to unlock phone");
                    break;
                default:
                Q_ASSERT(!"Unknown state");
                    break;
        };
    } else {
#ifdef QTOPIA_CELL
        // Sim lock
        bool waiting = false;
        switch(simLock->state()) {
            case BasicSimPinLock::VerifyingSimPuk:
            case BasicSimPinLock::VerifyingSimPin:
            case BasicSimPinLock::Waiting:
                text = tr("Please Wait...", "please wait while the phone checks to see if it needs a pin number, or if the most recently entered pin number is correct");
                waiting = true;
                break;
            case BasicSimPinLock::SimPinRequired:
                text = tr("Enter PIN then press %1<br>");
                break;
            case BasicSimPinLock::SimPukRequired:
                text= tr("Enter PUK then press %1<br>");
            break;
            case BasicSimPinLock::NewSimPinRequired:
                text= tr("Enter new PIN then press %1<br>");
            break;
            default:
                Q_ASSERT(!"Unknown state");
            break;
    }

        if(!waiting) {
            text.append(QString(simLock->number().length(), '*'));

            int lkey = BasicSimPinLock::lockKey();
                if (lkey == Qt::Key_Context1)
                    text = text.arg("Unlock");
                else
                    text = text.arg(QString(QKeySequence(lkey)));
            }
#endif // QTOPIA_CELL
            }

    if (lockMsgId) {
        clearInformation(lockMsgId);
        lockMsgId = 0;
    }

    if (!text.isEmpty()) {
        if (Qtopia::mousePreferred() && findItem("pinbox", Layout)) {
            // set here,
            ThemeTextItem *textItem = (ThemeTextItem *)findItem("pinbox", Text);
            if (textItem)
                textItem->setText(text);
            ThemeImageItem *imgItem = (ThemeImageItem *)findItem("pinbox", Image);
            if (imgItem) {
                imgItem->setImage( QPixmap( ":image/"+pix ) );
            }
            findItem("pinbox", Layout)->setActive(true);
        } else {
            lockMsgId = showInformation(pix, text, 0);
        }
    } else if (Qtopia::mousePreferred() && findItem("pinbox", Layout)) {
        findItem("pinbox", Layout)->setActive(false);
    }
}

#ifdef QTOPIA_PHONEUI
void HomeScreen::viewNewMessages()
{
    QtopiaServiceRequest req("SMS", "viewSms()");
    req.send();
}

void HomeScreen::viewMissedCalls()
{
    emit showMissedCalls();
}
#endif

void HomeScreen::inputMethodEvent(QInputMethodEvent *e)
{
    speeddialdown = true;
    QString ctext = e->commitString();
    QString ptext = e->preeditString();
    speeddial = speeddial.left(speeddial.length()-speeddial_preedit);
    if (!ctext.isEmpty()) {
        if (ctext != speeddial_activated_preedit) {
            speeddial += ctext;
            speeddial_preedit = 0;
        }
    }

    if (!ptext.isEmpty()) {
        speeddial += ptext;
        speeddial_preedit = ptext.length();
    }

    speeddial_activated_preedit.clear();

    if (!speeddial.isEmpty())
        speeddialTimer->start(1000);
}

void HomeScreen::specialButton(int keycode, bool held)
{
    const QDeviceButton* button =
        QDeviceButtonManager::instance().buttonForKeycode(keycode,"HomeScreen");
    if ( button ) {
        QtopiaServiceRequest sr;
        if ( held ) {
            sr = button->heldAction();
        } else {
            sr = button->pressedAction();
        }
        if ( !sr.isNull() )
            sr.send();
    }
}

bool HomeScreen::eventFilter(QObject *, QEvent *e)
{
#ifdef QTOPIA_PHONEUI
    bool calls = DialerControl::instance()->allCalls().count() != 0;
#else
    bool calls = false;
#endif

#ifdef QTOPIA_CELL
    bool locked = !calls && (keyLock->locked() || !simLock->open());
#else
    bool locked = !calls && (keyLock->locked());
#endif
    if(locked && e->type() == QEvent::KeyPress) {
        if(keyLock->locked())
            keyLock->processKeyEvent((QKeyEvent *)e);
#ifdef QTOPIA_CELL
        else
            simLock->processKeyEvent((QKeyEvent *)e);
#endif

        return true;
    } else if(!keyLock->locked()
#ifdef QTOPIA_CELL
              && simLock->open()
#endif
              && e->type() == QEvent::KeyPress) {
        keyLock->processKeyEvent((QKeyEvent *)e);
        return false;
    } else if(locked && e->type() == QEvent::KeyRelease) {
        return true;
    } else {
        return false;
    }
}

/* Reimplemented from ThemedView */
QWidget* HomeScreen::newWidget( ThemeWidgetItem* input, const QString& name )
{
    Q_UNUSED( input )

    // First check if the theme widget is a homescreen widget
    QWidget* widget = HomeScreenWidgets::create( name, this );
    if ( widget != 0 )
        return widget;

    // Now create any non-homescreen widgets

    return 0;
}

void HomeScreen::keyPressEvent(QKeyEvent *k)
{
    // also needs to respond to IM events....
    char ch = k->text()[0].toLatin1();
    if ((ch >= '0' && ch <= '9')
#ifdef QTOPIA_VOIP
        ||(ch >= 'a' && ch <= 'z')|| (ch >= 'A' && ch <= 'Z')
#endif
            || ch == 'p' || ch == 'P' || ch == '+' || ch == 'w'
            || ch == 'W' || ch == '#' || ch == '*' || ch == '@') {

#ifdef QTOPIA_PHONEUI
        if (DialerControl::instance()->isDialing()) {
            // send the digits as dtmf tones to the active calls.
            QList<QPhoneCall> activeCalls = DialerControl::instance()->activeCalls();
            if (activeCalls.count())
                activeCalls.first().tone(k->text());
            emit showCallScreen();
        } else {
            if ( !k->isAutoRepeat() ) {
                emit dialNumber(k->text().toLower());
            }
        }
#else
        if (ch >= '0' && ch <= '9' && !k->isAutoRepeat()) {
            speeddialdown = true;
            speeddial += ch;
            speeddialTimer->start(1000);
        }
#endif
        return;
    }
    switch (k->key()) {
#ifdef QTOPIA_PHONEUI
        case Qt::Key_Call:
        case Qt::Key_Yes:
            if (!DialerControl::instance()->hasIncomingCall() && !keyLocked() &&
                !DialerControl::instance()->isDialing())
                emit showCallHistory();
            break;
#endif
        case Qt::Key_Flip:
            {
            QSettings cfg("Trolltech","Phone");
            cfg.beginGroup("FlipFunction");
            if (!cfg.value("hangup").toBool())
                break;
            // (else FALL THROUGH)
            }
#ifdef QTOPIA_PHONEUI
        case Qt::Key_Hangup:
        case Qt::Key_No:
            if (DialerControl::instance()->allCalls().count())
                emit showCallScreen();
            DialerControl::instance()->endCall();
            break;
        case Qt::Key_Back:
            if (DialerControl::instance()->allCalls().count())
                emit showCallScreen();
            k->accept();
            break;
#else
        case Qt::Key_Back:
            k->accept();
            break;
#endif
        case Qt::Key_Select:
            k->accept();
            emit showPhoneBrowser();
            break;

        default:
            ph->filterDeviceButton(k->key(),true,k->isAutoRepeat());
            k->ignore();
    }

    if(!k->isAccepted())
        PhoneThemedView::keyPressEvent(k);
}

void HomeScreen::keyReleaseEvent(QKeyEvent *k)
{
    if ( !k->isAutoRepeat() )
        speeddialdown = false;
    if ( ph->filterDeviceButton(k->key(),false,k->isAutoRepeat()) )
        k->accept();
    else
        PhoneThemedView::keyReleaseEvent(k);
}

#ifdef QTOPIA_PHONEUI
void HomeScreen::showCallHistory(bool missed, const QString &/*hint*/)
{
    if (!DialerControl::instance()->hasIncomingCall() &&
        !DialerControl::instance()->isDialing()) {
        if(missed)
            emit showMissedCalls();
        else
            emit showCallHistory();
    }
}

void HomeScreen::phoneStateChanged()
{
    if (DialerControl::instance()->allCalls().count()) {
        QSoftMenuBar::setLabel(this, Qt::Key_Back, "phone/calls", tr("Calls"));
    } else {
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::NoLabel);
    }

    if (DialerControl::instance()->hasIncomingCall() ||
        DialerControl::instance()->isConnected() ||
        DialerControl::instance()->isDialing()) {

        if(keyLock->locked())
                setContextBarLocked(false);

        actionLock->setEnabled(false);
    } else {
        if(keyLock->locked())
            setContextBarLocked(true);

        actionLock->setEnabled(true);
    }
}
#endif

uint HomeScreen::showInformation(const QString &pix, const QString &text, int priority)
{
    static uint nextInfoId = 0;

    InfoData info;
    info.id = ++nextInfoId;
    info.priority = priority;
    info.text = text;
    info.pixmap = QPixmap(":image/"+pix);

    infoData.append(info);
    qSort(infoData);

    updateInfo();

    return info.id;
}

void HomeScreen::clearInformation(uint id)
{
    QList<InfoData>::Iterator it;
    for (it = infoData.begin(); it != infoData.end(); ++it) {
        if ((*it).id == (int)id) {
            infoData.erase(it);
            break;
        }
    }

    updateInfo();
}

void HomeScreen::updateInfo()
{
    bool hideInfo = true;
    ThemeTextItem *textItem;
    if (infoData.count()) {
        InfoData info = infoData[0];

        textItem = (ThemeTextItem *)findItem("infobox", Text);
        if (textItem)
            textItem->setText(info.text);
        ThemeImageItem *imgItem = (ThemeImageItem *)findItem("infobox", Image);
        if (imgItem)
            imgItem->setImage(info.pixmap);
        hideInfo = info.text.isEmpty();
    }

    ThemeItem *item = findItem("infobox", Item);
    if (item)
        item->setActive(!hideInfo);
}

#ifdef QTOPIA_CELL

void HomeScreen::cellBroadcast(CellBroadcastControl::Type type,
                               const QString &chan,
                               const QString &text)
{
    if(type == CellBroadcastControl::Background) {
        QString disp = chan + "\n" + text;
        infoMsgId = showInformation( "antenna", disp );
        QTimer::singleShot( 10000, this, SLOT(hideCBSMessage()));
        }
}

#endif // QTOPIA_CELL

#ifdef QTOPIA_PHONEUI
void HomeScreen::hideCBSMessage()
{
    clearInformation( infoMsgId );
}
#endif

HomeScreen *HomeScreen::getInstancePtr()
{
    static HomeScreen *homeScreen = 0;

    if (homeScreen)
        return homeScreen;

    homeScreen = new HomeScreen();
    return homeScreen;
}

#include "homescreen.moc"
