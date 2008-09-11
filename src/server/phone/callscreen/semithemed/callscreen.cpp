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

#include "callscreen.h"
#include "dialercontrol.h"
#include "qabstracthomescreen.h"
#include "servercontactmodel.h"
#include "qabstractmessagebox.h"
#include "videoringtone.h"
#include "qtopiainputevents.h"
#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QDialog>
#include <QItemDelegate>
#include <QKeyEvent>
#include <QLayout>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QPainter>
#include <QSettings>
#include <QTimer>
#include <QAbstractListModel>
#include <QDesktopWidget>
#include <QTextDocument>
#include <QList>
#include <QLabel>
#include <QProcess>

#include <QAudioStateConfiguration>
#include <QAudioStateInfo>
#include <qaudionamespace.h>
#include <QtopiaIpcEnvelope>

//#include <qexportedbackground.h>
#include <qtopiaservices.h>
#include <qsoftmenubar.h>
#include <qtopia/pim/qcontactmodel.h>
#include <qtopia/pim/qcontact.h>
#include <qtopianamespace.h>
#include <themedview.h>
#include <qphonecallmanager.h>

#include <QSimToolkit>

#include <QPhoneProfile>
#include <QPhoneProfileManager>

#include "delayedwaitdialog.h"

static const int  MAX_JOINED_CALLS = 5;
static const uint SECS_PER_HOUR= 3600;
static const uint SECS_PER_MIN  = 60;

#define SELECT_KEY_TIMEOUT 2000

static CallScreen *callScreen = 0;

class CallData {
public:
    CallData() {}
    CallData(const QPhoneCall &c) : call(c), callState(c.state()), havePhoto(false) {
        // Get the number or name to display in the text area.
        numberOrName = call.number();

        QContact cnt;
        QContactModel *m = ServerContactModel::instance();
        if (!call.contact().isNull()) {
            cnt = m->contact(call.contact());
        } else if (!numberOrName.isEmpty()) {
            cnt = m->matchPhoneNumber(numberOrName);
        }

        if (!cnt.uid().isNull()) {
            numberOrName = cnt.label();
            ringTone = cnt.customField( "tone" );
            QString pf = cnt.portraitFile();
            if( pf.isEmpty() ) {
                photo = ":image/addressbook/generic-contact.svg";
                havePhoto = false;
            } else {
                photo = Qtopia::applicationFileName( "addressbook", "contactimages/" ) + cnt.portraitFile();
                havePhoto = QFile::exists(photo);
            }
        }
    }

    bool isMulti() const {
        return ((callScreen->activeCallCount() > 1 && call.state() == QPhoneCall::Connected) ||
                (callScreen->heldCallCount() > 1 && call.onHold()));
    }

    QString durationString() {
        QString duration;
        if (!call.incoming() && !call.dialing()) {
            if (!connectTime.isNull()) {
                int elapsed;
                if (disconnectTime.isNull()) {
                    elapsed = connectTime.secsTo(QDateTime::currentDateTime());
                } else {
                    elapsed = connectTime.secsTo(disconnectTime);
                }
                int hour = elapsed/SECS_PER_HOUR;
                int minute = (elapsed % SECS_PER_HOUR)/SECS_PER_MIN;
                int second = elapsed % SECS_PER_MIN;
                QString buf;
                buf.sprintf( "%.2d:%.2d:%.2d", hour, minute, second );
                duration = buf;
            }
        }

        return duration;
    }

    QPhoneCall call;
    QPhoneCall::State callState;

    // QPixmap photo;
    // QString numberOrName;
    // elapsed time
    // conference
    // type

// what else should be exported in the vs

    QDateTime dialTime;
    QDateTime connectTime;
    QDateTime disconnectTime;
    QString ringTone;
    bool havePhoto;
    QString photo;
    QString numberOrName;
    QString state;
};

//===========================================================================

class SecondaryCallScreen : public QWidget
{
    Q_OBJECT
public:
    SecondaryCallScreen(QWidget *parent=0, Qt::WindowFlags f=0);

    void setCallData(const CallData &c) {
        callData = c;
        update();
    }

    const QPhoneCall &call() const { return callData.call; }

protected:
    void paintEvent(QPaintEvent *);

private:
    CallData callData;
};

SecondaryCallScreen::SecondaryCallScreen(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
}

void SecondaryCallScreen::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QRect r(rect());
    if (callData.havePhoto) {
        QPixmap photo(callData.photo);
        if (!photo.isNull()) {
            p.drawPixmap(0, 0, photo);
            r.adjust(photo.width(), 0, 0, 0);
        }
    }
    QString text(callData.numberOrName);
    QString subText;
    if (call().incoming() || call().dialing())
        subText = callData.state;
    else if (call().onHold())
        subText = callData.durationString() + QLatin1String(" ") + callData.state;
    else if (call().state() == QPhoneCall::Connected && !callScreen->incomingCall())
        subText = callData.durationString();
    else if( call().dropped() )
        subText = callData.state;
    else
        subText = callData.durationString();

    if (!subText.isEmpty())
        text += QLatin1String("<br><font size=-1>") + subText;
    QTextDocument doc;
    doc.setHtml("<b>"+text+"</b>");
    doc.setTextWidth(r.width());
    p.translate(r.x(), 0);
    doc.drawContents(&p);
}

//===========================================================================

/* declare CallItemListView */
class CallItemListView : public QListView
{
    Q_OBJECT
public:
    CallItemListView(ThemeWidgetItem *ti, QWidget *parent=0);
protected:
    void keyPressEvent(QKeyEvent*);
protected slots:
    void currentChanged(const QModelIndex &cur, const QModelIndex &prev);
private:
    CallScreen *mCallScreen;
    ThemeWidgetItem *thisThemeItem;
};

//-----------------------------------------------------------
/* define CallItemEntry */
class CallItemEntry : public ThemeListModelEntry
{
public:
    CallItemEntry(DialerControl *ctrl, const QPhoneCall &c, ThemeListModel* m);

    void setText(const QString & text) { display = text; }

    virtual QString type() const {
        QItemSelectionModel * selectModel = model()->listItem()->listView()->selectionModel();
        QModelIndex index = model()->entryIndex(this);
        if (selectModel->isSelected(index) && model()->listItem()->listView()->selectionMode() != QListWidget::NoSelection)
            return "selected";
        else if( call().incoming() )
            return "incoming";
        else if( call().dialing() )
            return "outgoing";
        else if( call().onHold() )
            return "onhold";
        else if( call().state() == QPhoneCall::Connected && !callScreen->incomingCall() )
            return "active";
        else if( call().dropped() )
            return "dropped";
        else
            return "active";
    }

    QPhoneCall &call() { return callData.call; }
    const QPhoneCall &call() const { return callData.call; }

    CallData callData;
    QString display;

private:
    DialerControl *m_control;
};

CallItemEntry::CallItemEntry(DialerControl *ctrl, const QPhoneCall &c, ThemeListModel* model)
    : ThemeListModelEntry(model),
            callData(c),
            m_control(ctrl)
{
}

//-----------------------------------------------------------
/* define CallItemModel */
typedef bool(*LessThan)(const ThemeListModelEntry *left, const ThemeListModelEntry *right);
class CallItemModel : public ThemeListModel
{
    Q_OBJECT
public:
    CallItemModel( QObject *parent, ThemeListItem* item, ThemedView* view) : ThemeListModel(parent, item, view) { };
    virtual ~CallItemModel() {}

    CallItemEntry* callItemEntry(const QModelIndex &index) const
    {
        QList<ThemeListModelEntry*> items = ThemeListModel::items();
        if (!index.isValid() || index.row() >= items.count())
            return 0;
        return static_cast<CallItemEntry*>(items.at(index.row()));
    }

    //sort items in list, reimplemented from QAbstractItemModel
    void sort(int column, Qt::SortOrder order)
    {
        if (column != 0)
            return;
        QList<ThemeListModelEntry*> items = ThemeListModel::items();
        LessThan compare = (order == Qt::AscendingOrder ? &itemLessThan : &itemGreaterThan);
        qSort(items.begin(), items.end(), compare);
        emit dataChanged(QAbstractListModel::index(0, 0), QAbstractListModel::index(items.count() - 1, 0));
    }

private:
    static bool itemLessThan(const ThemeListModelEntry *left, const ThemeListModelEntry* right)
    {
        return (static_cast<const CallItemEntry*>(left)->display) < (static_cast<const CallItemEntry*>(right)->display); //we use QString::compare - QString::localeAwareCompare not needed
    }

    static bool itemGreaterThan(const ThemeListModelEntry *left, const ThemeListModelEntry* right)
    {
        return !itemLessThan(left,right);
    }
};

//===================================================================

/* define CallItemListView */
CallItemListView::CallItemListView(ThemeWidgetItem *ti, QWidget *parent)
    : QListView(parent), mCallScreen( (CallScreen *)parent), thisThemeItem( ti )
{
    setResizeMode(QListView::Adjust);
}

void CallItemListView::keyPressEvent(QKeyEvent* ke)
{
    if ( ke->key() == Qt::Key_Flip ) {
        // QListWidget accepts too much
        ke->ignore();
    } else {
        QListView::keyPressEvent(ke);
    }
}

void CallItemListView::currentChanged(const QModelIndex &cur, const QModelIndex &prev)
{
    if ( selectionMode() != QAbstractItemView::SingleSelection )
        return;

    CallItemModel *m = qobject_cast<CallItemModel *>(model());
    if (!m)
        return;

    CallItemEntry *item = m->callItemEntry(cur);
    if (!item)
        return;
    item->setValue( "State", tr( " (Connect)",
                "describing an action to take on a call, make sure keeping the space in the beginning" ) );
    item = m->callItemEntry(prev);

    if (!item)
        return;
    item->setValue( "State", tr( " (Hold)",
                "describing an action to take on a call, make sure keeping the space in the beginning" ) );
}

//===========================================================================

/* declare CallAudioHandler */
class CallAudioHandler : public QObject
{
    Q_OBJECT
public:
    CallAudioHandler(QAudioStateConfiguration *conf, QObject *parent = 0);
    ~CallAudioHandler();

    void addOptionsToMenu(QMenu *menu);
    void callStateChanged(bool enableAudio);

private slots:
    void actionTriggered(QAction *action);
    void availabilityChanged();
    void currentStateChanged(const QAudioStateInfo &state, QAudio::AudioCapability capability);

private:
    QHash<QAction *, QAudioStateInfo> m_audioModes;
    QActionGroup *m_actions;
    bool m_audioActive;
    QtopiaIpcAdaptor *m_mgr;
    QAudioStateConfiguration *m_audioConf;
};

//-----------------------------------------------------------
/* define CallAudioHandler */

CallAudioHandler::CallAudioHandler(QAudioStateConfiguration *conf, QObject *parent)
	: QObject(parent)
{
    m_mgr = new QtopiaIpcAdaptor("QPE/AudioStateManager", this);
    m_audioConf = conf;

    m_audioActive = false;

    connect(conf, SIGNAL(currentStateChanged(QAudioStateInfo,QAudio::AudioCapability)),
            this, SLOT(currentStateChanged(QAudioStateInfo,QAudio::AudioCapability)));
    connect(conf, SIGNAL(availabilityChanged()),
            this, SLOT(availabilityChanged()));

    QSet<QAudioStateInfo> states = conf->states("Phone");

    m_actions = new QActionGroup(this);

    foreach (QAudioStateInfo state, states) {
        QAction *action = new QAction(state.displayName(), this);
        action->setVisible(false);
        action->setCheckable(true);
        m_actions->addAction(action);
        m_audioModes.insert(action, state);
    }

    connect(m_actions, SIGNAL(triggered(QAction*)),
            this, SLOT(actionTriggered(QAction*)));
}

CallAudioHandler::~CallAudioHandler()
{
}

void CallAudioHandler::addOptionsToMenu(QMenu *menu)
{
    foreach (QAction *action, m_audioModes.keys()) {
	menu->addAction(action);
    }
}

void CallAudioHandler::actionTriggered(QAction *action)
{
    if (!m_audioActive) {
        qWarning("CallAudioHandler::actionTriggered while audio is not active!");
        return;
    }

    if (!m_audioModes.contains(action)) {
        qWarning("CallAudioHandler::actionTriggered - Invalid action!");
        return;
    }

    m_mgr->send("setProfile(QByteArray)", m_audioModes[action].profile());
}

void CallAudioHandler::availabilityChanged()
{
    if (!m_audioActive)
        return;

    QHash<QAction *, QAudioStateInfo>::const_iterator it = m_audioModes.constBegin();
    while (it != m_audioModes.constEnd()) {
        bool vis = m_audioConf->isStateAvailable(m_audioModes[it.key()]);
        it.key()->setVisible(vis);
        ++it;
    }
}

void CallAudioHandler::currentStateChanged(const QAudioStateInfo &state, QAudio::AudioCapability)
{
    if (!m_audioActive)
        return;

    QHash<QAction *, QAudioStateInfo>::const_iterator it = m_audioModes.constBegin();
    while (it != m_audioModes.constEnd()) {
        if (it.value() == state) {
            it.key()->setChecked(true);
            return;
        }
        ++it;
    }
}


void CallAudioHandler::callStateChanged(bool enableAudio)
{
    if (enableAudio == m_audioActive)
        return;

    m_audioActive = enableAudio;

    if (m_audioActive) {
        QByteArray domain("Phone");
        int capability = static_cast<int>(QAudio::OutputOnly);
        m_mgr->send("setDomain(QByteArray,int)", domain, capability);

        QtopiaIpcEnvelope e("QPE/AudioVolumeManager", "setActiveDomain(QString)");
        e << QString("Phone");

        availabilityChanged();
    }
    else {
        foreach (QAction *action, m_actions->actions()) {
            action->setChecked(false);
            action->setVisible(false);
        }

        //TODO: This needs to be fixed  up later to send the release
        // domain message instead
        QByteArray domain("Media");
        int capability = static_cast<int>(QAudio::OutputOnly);
        m_mgr->send("setDomain(QByteArray,int)", domain, capability);

        QtopiaIpcEnvelope e("QPE/AudioVolumeManager", "resetActiveDomain(QString)");
        e << QString("Phone");
    }
}

//===========================================================================
CallScreenView::CallScreenView(QWidget* parent)
    : PhoneThemedView(parent)
{}

/* Reimplemented from ThemedView */
/*!
  \internal
  */
QWidget *CallScreenView::newWidget(ThemeWidgetItem* input, const QString& name)
{
    if( name == "callscreen" )  {
        Q_ASSERT(input->rtti() == ThemedView::List);
        CallItemListView * lv = new CallItemListView( input, this );
        if(callScreen->m_model != 0)
            delete callScreen->m_model;
        callScreen->m_model = new CallItemModel( this, static_cast<ThemeListItem*>(input), this );
        lv->setModel(callScreen->m_model);
        return lv;
    }
    return 0;
}

void CallScreenView::themeLoaded(const QString& fn)
{
    emit themeWasLoaded(fn);
}



//===========================================================================

/*!
  \class CallScreen
  \brief The CallScreen class provides a phone call screen.
  \ingroup QtopiaServer::PhoneUI

  This class is part of the Qtopia server and cannot be used by other Qtopia applications.
  */


/*!
  \fn void CallScreen::acceptIncoming()
  \internal
  */

/*!
  \fn int CallScreen::activeCallCount() const
  \internal
  */

/*!
  \fn int CallScreen::heldCallCount() const
  \internal
  */


/*!
    \fn bool CallScreen::incomingCall() const
    \internal
    */

/*!
    \fn bool CallScreen::inMultiCall() const
    \internal
    */

/*!
  \fn void CallScreen::muteRing()
  \internal
  */

/*!
  \fn void CallScreen::listEmpty()
  \internal
  */

/*!
    \fn void CallScreen::testKeys(const QString&, bool&)
    \internal
    */

/*!
  \fn void CallScreen::filterKeys(const QString&, bool&)
  \internal
  */

/*!
  \fn void CallScreen::filterSelect(const QString&, bool&)
  \internal
  */

/* define CallScreen */
/*!
  \internal
  */
CallScreen::CallScreen(DialerControl *ctrl, QWidget *parent, Qt::WFlags fl)
    : QWidget(parent, fl)
    , m_listView(0)
    , m_model(0)
    , m_digits(0)
    , m_actionGsm(0)
    , m_control(ctrl)
    , m_activeCount(0)
    , m_holdCount(0)
    , m_updateTimer( 0 )
    , m_gsmActionTimer(0)
    , m_callAudioHandler(0)
    , m_simMsgBox(0)
    , m_symbolTimer(0)
    , m_hadActiveCall(false)
{
    callScreen = this;
    setObjectName(QLatin1String("calls"));

    // ui bits
    m_view = new CallScreenView(this);
    connect(m_view, SIGNAL(itemReleased(ThemeItem*)),
            SLOT(themeItemReleased(ThemeItem*)));
    connect(m_view, SIGNAL(themeWasLoaded(const QString&)),
            SLOT(themeLoaded(const QString&)));

    m_digits = new QLineEdit(this);
    QFont font = m_digits->font();
    font.setPointSize(12);
    font.setWeight(QFont::Bold);
    m_digits->setFont(font);
    m_digits->setFrame(false);
    m_digits->setReadOnly(true);
    m_digits->setFocusPolicy(Qt::NoFocus);
    connect(m_digits, SIGNAL(textChanged(QString)),
            this, SLOT(updateLabels()) );

    QVBoxLayout* lay = new QVBoxLayout(this);
    lay->setMargin(0);
    lay->addWidget(m_view);
    lay->addWidget(m_digits);
    

    m_contextMenu = QSoftMenuBar::menuFor(this);

    m_actionAnswer = new QAction(QIcon(":icon/phone/answer"),tr("Answer", "answer call"), this);
    connect(m_actionAnswer, SIGNAL(triggered()), this, SIGNAL(acceptIncoming()));
    m_actionAnswer->setVisible(false);
    m_contextMenu->addAction(m_actionAnswer);

    m_actionSendBusy = new QAction(QIcon(":icon/phone/reject"), tr("Send Busy"), this);
    connect(m_actionSendBusy, SIGNAL(triggered()), m_control, SLOT(sendBusy()));
    m_actionSendBusy->setVisible(false);
    m_contextMenu->addAction(m_actionSendBusy);

    m_actionMute = new QAction(QIcon(":icon/mute"),tr("Mute"), this);
    connect(m_actionMute, SIGNAL(triggered()), this, SLOT(muteRingSelected()));
    m_actionMute->setVisible(false);
    m_contextMenu->addAction(m_actionMute);

    m_actionEnd = new QAction(QIcon(":icon/phone/hangup"),tr("End"),this);
    connect(m_actionEnd, SIGNAL(triggered()), m_control, SLOT(endCall()));
    m_actionEnd->setVisible(false);
    m_contextMenu->addAction(m_actionEnd);

    m_actionEndAll = new QAction(tr("End all calls"), this);
    connect(m_actionEndAll, SIGNAL(triggered()), m_control, SLOT(endAllCalls()));
    m_actionEndAll->setVisible(false);
    m_contextMenu->addAction(m_actionEndAll);

    m_actionHold = new QAction(QIcon(":icon/phone/hold"),tr("Hold"), this);
    connect(m_actionHold, SIGNAL(triggered()), m_control, SLOT(hold()));
    m_actionHold->setVisible(false);
    m_contextMenu->addAction(m_actionHold);

    m_actionResume = new QAction(QIcon(":icon/phone/resume"),tr("Resume"), this);
    connect(m_actionResume, SIGNAL(triggered()), m_control, SLOT(unhold()));
    m_actionResume->setVisible(false);
    m_contextMenu->addAction(m_actionResume);

    m_actionMerge = new QAction(QIcon(":icon/phone/conference"),tr("Join"), this);
    connect(m_actionMerge, SIGNAL(triggered()), m_control, SLOT(join()));
    m_actionMerge->setVisible(false);
    m_contextMenu->addAction(m_actionMerge);

    m_actionSplit = new QAction(tr("Split..."), this);
    connect(m_actionSplit, SIGNAL(triggered()), this, SLOT(splitCall()));
    m_actionSplit->setVisible(false);
    m_contextMenu->addAction(m_actionSplit);

    m_actionTransfer = new QAction(QIcon(":icon/phone/callforwarding"),tr("Transfer"),this);
    connect(m_actionTransfer, SIGNAL(triggered()), m_control, SLOT(transfer()));
    m_actionTransfer->setVisible(false);
    m_contextMenu->addAction(m_actionTransfer);

    m_audioConf = new QAudioStateConfiguration(this);

    if (m_audioConf->isInitialized())
        initializeAudioConf();
    else
        QObject::connect(m_audioConf, SIGNAL(configurationInitialized()),
                         this, SLOT(initializeAudioConf()));

    setWindowTitle(tr("Calls"));

    QObject::connect(m_control,
                     SIGNAL(requestFailed(QPhoneCall,QPhoneCall::Request)),
                     this,
                     SLOT(requestFailed(QPhoneCall,QPhoneCall::Request)));

    QObject::connect(m_control, SIGNAL(callConnected(const QPhoneCall&)),
                     this, SLOT(callConnected(const QPhoneCall&)));

    QObject::connect(m_control, SIGNAL(callDropped(const QPhoneCall&)),
                     this, SLOT(callDropped(const QPhoneCall&)));

    QObject::connect(m_control, SIGNAL(callDialing(const QPhoneCall&)),
                     this, SLOT(callDialing(const QPhoneCall&)));

    // reject any dialogs when new call coming in
    QObject::connect(m_control, SIGNAL(callIncoming(const QPhoneCall&)),
                     this, SLOT(rejectModalDialog()));
    QObject::connect(m_control, SIGNAL(callIncoming(const QPhoneCall&)),
                     this, SLOT(callIncoming(const QPhoneCall&)));

    m_simToolkit = new QSimToolkit( QString(), this );
    QObject::connect( m_simToolkit, SIGNAL(controlEvent(QSimControlEvent)),
            this, SLOT(simControlEvent(QSimControlEvent)) );

    // Due to delayed intialization of call screen
    // manual update on incoming call is required when call screen is created
    if ( m_control->hasIncomingCall() )
        callIncoming( m_control->incomingCall() );
}

void CallScreen::initializeAudioConf()
{
    // add speaker, bluetooth headset actions, etc
    m_callAudioHandler = new CallAudioHandler(m_audioConf, this);
    m_callAudioHandler->addOptionsToMenu(m_contextMenu);
}


/*!
  \internal
  */
bool CallScreen::dialNumbers(const QString & numbers)
{
    // allow to enter '+' symbol by pressing '*' key twice quickly
    // required when an internationl number is entered
    if ( numbers == "*" ) {
        if ( m_dtmfDigits.endsWith( "*" )
                && m_symbolTimer->isActive() ) {
            m_dtmfDigits = m_dtmfDigits.left( m_dtmfDigits.length() - 1 );
            appendDtmfDigits( "+" );
            return true;
        } else {
            if ( !m_symbolTimer ) {
                m_symbolTimer = new QTimer( this );
                m_symbolTimer->setSingleShot( true );
            }
            m_symbolTimer->start( 500 );
        }
    }

    if (m_control->isDialing() && !m_control->hasActiveCalls()) {
        appendDtmfDigits(numbers);
        return true;
    } else if (m_control->hasActiveCalls()) {
        // Inject the specified m_digits into the display area.
        m_control->activeCalls().first().tone(numbers);
        appendDtmfDigits(numbers);
        return true;
    } else if ( m_control->hasIncomingCall() ) {
        appendDtmfDigits(numbers);
    } else if ( m_control->hasCallsOnHold() ) {
        appendDtmfDigits(numbers);
        return true;
    }
    return false;
}

/*!
  \internal
  */
void CallScreen::themeLoaded( const QString & )
{
    ThemeWidgetItem *item = 0;
    item = (ThemeWidgetItem*)m_view->findItem( "callscreen", ThemedView::List, ThemeItem::All, false );
    if( !item ) {
        qWarning("No callscreen element defined for CallScreen theme.");
        m_listView = 0;
        return;
    }

    m_listView = qobject_cast<CallItemListView *>(item->widget());
    Q_ASSERT(m_listView != 0 );
    // active item
    // hold item
    // joined call indicator
    // contact image
    // phone number text
    // status text

    m_listView->setFrameStyle(QFrame::NoFrame);
    m_listView->installEventFilter(this);
    m_listView->setSelectionMode(QAbstractItemView::NoSelection);
    m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_listView, SIGNAL(activated(QModelIndex)), this, SLOT(callSelected(QModelIndex)));
    QSoftMenuBar::setLabel(m_listView, Qt::Key_Select, QSoftMenuBar::NoLabel);


    stateChanged();
}

/*!
  \internal
  */
QString CallScreen::ringTone()
{
    if (!m_listView)
        return QString();

    CallItemModel* m = qobject_cast<CallItemModel *>(m_listView->model());
    for (int i = m->rowCount()-1; i>=0; i--) {
        CallItemEntry* item = m->callItemEntry(m->index(i));
        if ( item && item->callData.callState == QPhoneCall::Incoming ) {
            return item->callData.ringTone;
        }
    }
    return QString();
}

void CallScreen::makeVisible()
{
    if(!m_view->sourceLoaded())
        m_view->loadSource();

    showMaximized();
    raise();
    activateWindow();
}

/*!
  \internal
  */
void CallScreen::clearDtmfDigits(bool clearOneChar)
{
    if(m_dtmfDigits.isEmpty())
        return;

    if (clearOneChar)
        m_dtmfDigits = m_dtmfDigits.left(m_dtmfDigits.length() - 1);
    else
        m_dtmfDigits.clear();

    m_digits->setText(m_dtmfDigits);

    if (m_dtmfDigits.isEmpty()) {
        updateLabels();
    } else if (m_gsmActionTimer) {
        m_gsmActionTimer->start();
    }

    if (m_listView) {
        CallItemModel* m = qobject_cast<CallItemModel *>(m_listView->model());
        m->triggerUpdate();
    }

    // remove menu item
    setGsmMenuItem();
}

/*!
  \internal
  */
void CallScreen::setGsmMenuItem()
{
    if (!m_actionGsm) {
        m_actionGsm = new QAction(QIcon(":icon/phone/answer"),QString(), this);
        connect(m_actionGsm, SIGNAL(triggered()), this, SLOT(actionGsmSelected()));
        QSoftMenuBar::menuFor(this)->addAction(m_actionGsm);
    }

    bool filterable = false;
    emit testKeys( m_dtmfDigits, filterable );

    m_actionGsm->setVisible(!m_dtmfDigits.isEmpty());

    // update menu text & lable for Key_Select
    if (!m_dtmfDigits.isEmpty() && m_listView) {
        if (filterable) {
            m_actionGsm->setText(tr("Send %1").arg(m_dtmfDigits));
            QSoftMenuBar::setLabel(m_listView, Qt::Key_Select, "", tr("Send"));
        } else {
            m_actionGsm->setText(tr("Call %1", "%1=phone number").arg(m_dtmfDigits));
            QSoftMenuBar::setLabel(m_listView, Qt::Key_Select, "phone/answer", tr("Call"));
        }
    }
}

/*!
  \internal
  */
void CallScreen::actionGsmSelected()
{
    bool filtered = false;
    emit filterSelect(m_dtmfDigits, filtered);
    // if the m_digits are not filtered place a call
    if ( !filtered ) {
        // check if contact exists
        QContactModel *m = ServerContactModel::instance();
        QContact cnt = m->matchPhoneNumber(m_dtmfDigits);

        if ( cnt == QContact() ) { // no contact
            QtopiaServiceRequest service( "Dialer", "dial(QString,QString)" );
            service << QString() << m_dtmfDigits;
            service.send();
        } else {
            QtopiaServiceRequest service( "Dialer", "dial(QString,QUniqueId)" );
            service << m_dtmfDigits << cnt.uid();
            service.send();
        }
    }
    // clear m_digits wheather filtered or not
    clearDtmfDigits();
}

/*!
  \internal
  */
void CallScreen::updateLabels()
{
    // update context label according to the current call count.
    if (m_listView) {
        if (m_control->allCalls().count() >= 2)
            QSoftMenuBar::setLabel(m_listView, Qt::Key_Select, "phone/swap", tr("Swap"));
        else if (m_control->activeCalls().count() == 1)
            QSoftMenuBar::setLabel(m_listView, Qt::Key_Select, "phone/hold", tr("Hold"));
        else
            QSoftMenuBar::setLabel(m_listView, Qt::Key_Select, QSoftMenuBar::NoLabel);
    }

    // display clear icon when dtmf m_digits are entered.
    if (!m_digits->text().isEmpty())
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace);
    else
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Back);
}

/*!
  \internal
  */
void CallScreen::appendDtmfDigits(const QString &dtmf)
{
    m_dtmfDigits.append(dtmf);
    if(m_dtmfDigits.isEmpty())
        return;

    m_digits->setText(m_dtmfDigits);
    m_digits->setCursorPosition(m_digits->text().length());

    if (m_listView) {
        CallItemModel* m = qobject_cast<CallItemModel *>(m_listView->model());
        m->triggerUpdate();
    }

    // add menu item.
    setGsmMenuItem();

    if (!m_gsmActionTimer) {
        m_gsmActionTimer = new QTimer(this);
        m_gsmActionTimer->setInterval(SELECT_KEY_TIMEOUT);
        m_gsmActionTimer->setSingleShot(true);
        QObject::connect(m_gsmActionTimer, SIGNAL(timeout()), this, SLOT(updateLabels()));
    }
    m_gsmActionTimer->start();

    // filter immediate m_action
    bool filtered = false;
    emit filterKeys( m_dtmfDigits, filtered );
    if ( filtered ) {
        clearDtmfDigits();
    }
}

/*!
  \internal
  */
void CallScreen::stateChanged()
{
    if( !m_listView )
        return;
    const QList<QPhoneCall> &calls = m_control->allCalls();


    // see if any calls have ended.

    CallItemEntry *item = 0;
    CallItemModel *m = qobject_cast<CallItemModel *>(m_listView->model());
    for (int i = m->rowCount()-1; i>=0; i--) {
        item = m->callItemEntry(m->index(i));
        if (item && !calls.contains(item->call())) {
            if (item->callData.disconnectTime.isNull()) {
                item->callData.disconnectTime = QDateTime::currentDateTime();
                item->setText("f");
            }
        }
    }

    m_activeCount = 0;
    m_holdCount = 0;
    m_incoming = false;
    bool dialing = false;

    CallItemEntry *active = 0;
    CallItemEntry *primaryItem = 0;
    int primaryOrder = 9;

    // any calls added/changed state
    int idx = 1;
    QList<QPhoneCall>::ConstIterator it;
    QString name;
    bool itemStateChanged = false;
    QString state;

    for (it = calls.begin(); it != calls.end(); ++it, idx++) {
        int sortOrder = 9;
        const QPhoneCall &call(*it);
        item = findCall(call, m);
        if (!item) {
            // The modem has no identifier for us. There will be another state
            // changed signal and then we can start showing it to the user.
            if (call.modemIdentifier() == -1)
                continue;

            item = new CallItemEntry(m_control, call, m);
            m->addEntry(item);
        }
        if (item->callData.connectTime.isNull() && call.established())
            item->callData.connectTime = QDateTime::currentDateTime();
        //m->triggerUpdate();

        name = item->callData.numberOrName;
        if (name.isEmpty())
            name = CallScreen::tr("Unknown caller");

        if( call.state() != item->callData.callState ) {
            item->callData.callState = call.state();
            itemStateChanged = true;
        }
        if (call.state() == QPhoneCall::Connected) {
            m_activeCount++;
            if (!active)
                active = item;
            sortOrder = 2;
            state = CallScreen::tr("Connected", "call state");
            if ( m_simMsgBox && m_simMsgBox->isVisible() )
                m_simMsgBox->hide();
        } else if (call.state() == QPhoneCall::Hold) {
            m_holdCount++;
            sortOrder = 3;
            state = CallScreen::tr("Hold", "call state");
        } else if (call.state() == QPhoneCall::Dialing ||
                   call.state() == QPhoneCall::Alerting) {
            dialing = true;
            if (!active)
                active = item;
            sortOrder = 1;
            item->callData.callState = QPhoneCall::Dialing;
            item->callData.dialTime = QDateTime::currentDateTime();
            state = CallScreen::tr("Dialing", "call state");
        } else if (call.state() == QPhoneCall::Incoming) {
            sortOrder = 0;
            m_incoming = true;
            item->callData.callState = QPhoneCall::Incoming;
            state = CallScreen::tr("Incoming", "call state");
        }
        item->setText(QChar('a'+sortOrder) + QString::number(idx));
        item->callData.state = state;

        bool isMulti = ((callScreen->activeCallCount() > 1 && call.state() == QPhoneCall::Connected) ||
                (callScreen->heldCallCount() > 1 && call.onHold()));
        item->setValue( "State", tr( " (%1)", "describing call state, make sure keeping the space in the beginning" ).arg(state) );
        item->setValue( "CallId", tr( "(%1) ", "describing call id, make sure keeping the space int the end" ).arg( call.modemIdentifier() ) );
        item->setValue( "Identifier", name );
        item->setValue( "Conference", isMulti );

        if (sortOrder < primaryOrder) {
            primaryItem = item;
            primaryOrder = sortOrder;
        }
    }

    for (int i = m->rowCount()-1; i>=0; i--) {
        if (!m->index(i).isValid())
            break;
        CallItemEntry* item = m->callItemEntry(m->index(i));
        if( !item )
            continue;
        if( item->call().dropped() ) {
            item->setValue("State", tr( " (Disconnected)", "describing call state, make sure keeping the space in the beginning") );
        }
    }
    
    // Get the current profile
    QPhoneProfileManager* profileManager = new QPhoneProfileManager(this);
    QPhoneProfile currentProfile = profileManager->activeProfile();

    // update available m_actions.
    m_actionAnswer->setVisible(m_control->hasIncomingCall());
    m_actionSendBusy->setVisible(m_control->hasIncomingCall() && currentProfile.volume() > 0);
    m_actionMute->setVisible(m_control->hasIncomingCall());
    m_actionHold->setVisible(m_activeCount && !m_holdCount && !m_incoming && !dialing);
    m_actionResume->setVisible(m_holdCount && !m_incoming && !dialing);
    m_actionEnd->setVisible((m_activeCount || m_holdCount || dialing) && !m_incoming);
    m_actionEndAll->setVisible(m_activeCount && m_holdCount && !m_incoming);
    m_actionMerge->setVisible(m_activeCount && m_holdCount &&
                            m_activeCount < MAX_JOINED_CALLS &&
                            m_holdCount < MAX_JOINED_CALLS && !m_incoming);
    m_actionSplit->setVisible(m_activeCount > 1 && !m_holdCount && !m_incoming);
    m_actionTransfer->setVisible(m_activeCount == 1 && m_holdCount == 1 && !m_incoming);

    // Update the content based on our action state
    setItemActive("hold", m_actionHold->isVisible());
    setItemActive("resume", m_actionResume->isVisible());

    // update the speaker and bluetooth headset m_actions.
    bool nonActiveDialing = dialing && !m_activeCount;

    if (m_callAudioHandler)
        m_callAudioHandler->callStateChanged(m_activeCount || m_holdCount || nonActiveDialing /* || m_incoming*/ );

    if (m_incoming) {
        QSoftMenuBar::setLabel(m_listView, Qt::Key_Select, "phone/answer", tr("Answer"));
    } else if (m_activeCount && m_holdCount) {
        setItemText("resume_text", tr("Swap"));
        m_actionResume->setText(tr("Swap", "change to 2nd open phoneline and put 1st on hold"));
        m_actionResume->setIcon(QPixmap(":icon/phone/swap"));
        QSoftMenuBar::setLabel(m_listView, Qt::Key_Select, "phone/swap", tr("Swap"));
    } else if (m_holdCount && !m_activeCount && !dialing && !m_incoming) {
        setItemText("resume_text", tr("Resume"));
        m_actionResume->setText(tr("Resume"));
        m_actionResume->setIcon(QIcon(":icon/phone/resume"));
        QSoftMenuBar::setLabel(m_listView, Qt::Key_Select, "phone/resume", tr("Resume"));
    } else if (m_activeCount && !m_holdCount && !dialing && !m_incoming) {
        QSoftMenuBar::setLabel(m_listView, Qt::Key_Select, "phone/hold", tr("Hold"));
    } else {
        QSoftMenuBar::setLabel(m_listView, Qt::Key_Select, QSoftMenuBar::NoLabel);
    }

    if (m_incoming && m_listView->selectionMode() != QAbstractItemView::SingleSelection)
        if ( m_incoming )
            QSoftMenuBar::setLabel(m_listView, Qt::Key_Back, ":icon/mute", tr("Mute"));
        else
            QSoftMenuBar::setLabel(m_listView, Qt::Key_Back, QSoftMenuBar::NoLabel);
    else
        QSoftMenuBar::clearLabel(m_listView, Qt::Key_Back);

    //layout()->activate();
    m->sort(0, Qt::AscendingOrder);
    if (m->rowCount() > 0)
        m_listView->scrollTo(m->index(0));

    if( itemStateChanged ) // any items state changed?
        updateAll();
    else
        m->triggerUpdate();

    // Check dtmf status
    if(!active || active->call().identifier() != m_dtmfActiveCall)
        clearDtmfDigits();
    if(active)
        m_dtmfActiveCall = active->call().identifier();
    else
        m_dtmfActiveCall = QString();
    update();
    
    // If there is no ring tone, we skip the muteRing action
    if (currentProfile.volume() == 0)
        muteRingSelected();
}

/*!
  \internal
  */
void CallScreen::requestFailed(const QPhoneCall &,QPhoneCall::Request r)
{
    QString title, text;
    title = tr("Failure");
    if(r == QPhoneCall::HoldFailed) {
        text = tr("Hold attempt failed");
    } else if(r == QPhoneCall::ActivateFailed) {
        text = tr("Activate attempt failed");
    } else {
        text = tr("Join/transfer attempt failed");
    }

    QAbstractMessageBox *box = QAbstractMessageBox::messageBox(0, title, text, QAbstractMessageBox::Warning);
    box->setTimeout(3000, QAbstractMessageBox::NoButton);
    QtopiaApplication::execDialog(box);
    delete box;
}

/*!
  \internal
  */
CallItemEntry *CallScreen::findCall(const QPhoneCall &call, CallItemModel *m)
{
    CallItemEntry *item = 0;
    for (int i = m->rowCount()-1; i>=0; i--) {
        item = m->callItemEntry(m->index(i));
        if (item->call() == call)
            return item;
    }

    return 0;
}

/*!
  \internal
  */
void CallScreen::updateAll()
{
    updateAll(false);
}

/*!
  \internal
  */
void CallScreen::updateAll(bool force)
{
    if( !m_listView )
        return;

    CallItemModel *m = qobject_cast<CallItemModel *>(m_listView->model());
    for (int i = m->rowCount()-1; i>=0; i--) {
        if (!m->index(i).isValid())
            break;
        CallItemEntry* item = m->callItemEntry(m->index(i));
        if (item->call().state() == QPhoneCall::ServiceHangup) {
            // USSD message is coming soon, so remove from the call screen.
            m->removeEntry(m->index(i)); // removeEntry will delete the item
            i--;
            continue;
        } else if (item->call().dropped()) {
            // Remove dropped calls after a short delay
            if (force
                || (!item->callData.disconnectTime.isNull() &&
                item->callData.disconnectTime.time().elapsed() > 3000)) {
                m->removeEntry(m->index(i)); // removeEntry will delete the item
                i--;
                continue;
            }
        }

        /* Set appropriate information in the CallItemModel item */
        item->setValue( "Duration", item->callData.durationString() );
        if( item->value("Photo").toString().isEmpty() )
            item->setValue( "Photo", item->callData.photo );
    }

    if (m->rowCount() == 0) {
        emit listEmpty();
    } else {
        m->triggerUpdate();
    }
}

/*!
  \internal
  */
void CallScreen::splitCall()
{
    setWindowTitle(tr("Select Call...","split 2 phone lines after having joined them"));
    setSelectMode(true);
    m_actionHold->setVisible(false);
    m_actionResume->setVisible(false);
    m_actionEnd->setVisible(false);
    m_actionEndAll->setVisible(false);
    m_actionMerge->setVisible(false);
    m_actionSplit->setVisible(false);
    m_actionTransfer->setVisible(false);
}

/*!
  \internal
  */
void CallScreen::callSelected(const QModelIndex& index)
{
    CallItemModel* m = qobject_cast<CallItemModel *>(m_listView->model());

    CallItemEntry *callItem = m->callItemEntry(index);
    if (!callItem )
        qWarning("CallScreen::callSelected(): invalid index passed to CallItemModel");
    if (m->flags(index) & Qt::ItemIsSelectable) {
        if (callScreen->heldCallCount() || callScreen->activeCallCount() > 1) {
            setSelectMode(false);
            //XXX I could be used for more than just split
            callItem->call().activate(QPhoneCall::CallOnly);
            setWindowTitle(tr("Calls"));
        } else if (!Qtopia::mousePreferred()){ // this is the only call
            if (callItem->call().onHold())
                m_control->unhold();
            else if (callItem->call().connected())
                m_control->hold();
        }
    }
}

/*!
  \internal
  */
void CallScreen::setItemActive(const QString &name, bool active)
{
    ThemeItem *item = m_view->findItem(name, ThemedView::Item, ThemeItem::All, false);
    if (item)
        item->setActive(active);
}

/*!
  \internal
  */
void CallScreen::setItemText(const QString &name, const QString& text)
{
    ThemeItem *item = m_view->findItem(name, ThemedView::Text, ThemeItem::All, false);
    if (item && item->rtti() == ThemedView::Text)
        static_cast<ThemeTextItem*>(item)->setText(text);
}

/*!
  \internal
  */
void CallScreen::themeItemReleased(ThemeItem *item)
{
    if (!item)
        return;

    if (item->itemName() == "answer")
    {
        m_actionAnswer->trigger();
    }
    else if (item->itemName() == "endcall")
    {
        m_actionEnd->trigger();
    }
    else if (item->itemName() == "hold")
    {
        if (!m_control->hasActiveCalls())
            return;

        m_actionHold->trigger();
    }
    else if (item->itemName() == "resume")
    {
        if (!m_control->hasCallsOnHold())
            return;

        m_actionResume->trigger();
    }
    else if (item->itemName() == "sendbusy")
    {
        m_actionSendBusy->trigger();
    }
    else if (item->itemName() == "show_keypad")
    {
        static QHash<QString, bool> showKeypad;
        if (showKeypad.isEmpty()) {
            showKeypad["menu-box"]   = false;
            showKeypad["keypad-box"] = true;
        }

        m_view->setActiveItems(showKeypad);
    }
    else if (item->itemName() == "hide_keypad")
    {
        static QHash<QString, bool> hideKeypad;
        if (hideKeypad.isEmpty()) {
            hideKeypad["menu-box"]   = true;
            hideKeypad["keypad-box"] = false;
        }

        m_view->setActiveItems(hideKeypad);
    }
    else if ( item->itemName() == "zero" )
    {
        dialNumbers("0");
    }
    else if(  item->itemName() == "one" )
    {
        dialNumbers("1");
    }
    else if ( item->itemName() == "two" )
    {
        dialNumbers("2");
    }
    else if ( item->itemName() == "three" )
    {
        dialNumbers("3");
    }
    else if ( item->itemName() == "four" )
    {
        dialNumbers("4");
    }
    else if ( item->itemName() == "five" )
    {
        dialNumbers("5");
    }
    else if ( item->itemName() == "six" )
    {
        dialNumbers("6");
    }
    else if ( item->itemName() == "seven" )
    {
        dialNumbers("7");
    }
    else if ( item->itemName() == "eight" )
    {
        dialNumbers("8");
    }
    else if ( item->itemName() == "nine" )
    {
        dialNumbers("9");
    }
    else if ( item->itemName() == "star" )
    {
        dialNumbers("*");
    }
    else if ( item->itemName() == "hash" )
    {
        dialNumbers("#");
    }
}

/*!
  \internal
  */
void CallScreen::keyPressEvent(QKeyEvent *k)
{
    if (k->key() == Qt::Key_Flip) {
        QSettings cfg("Trolltech","Phone");
        cfg.beginGroup("FlipFunction");
        if (cfg.value("hangup").toBool()) {
            m_control->endAllCalls();
            hide();
        }
    } else if (k->key() == Qt::Key_Hangup || k->key() == Qt::Key_No) {
        if (m_control->isConnected() || m_control->isDialing() || m_control->hasIncomingCall())
            m_control->endCall();
        else
            hide();
    } else if ((k->key() == Qt::Key_F28) && m_control->isConnected() && !m_control->hasIncomingCall()) {
        m_control->endCall();
    } else if (k->key() == Qt::Key_Call || k->key() == Qt::Key_Yes || k->key() == Qt::Key_F28) {
        if (!m_dtmfDigits.isEmpty()) {
            actionGsmSelected();
        } else {
            if ( m_control->hasIncomingCall() )
                emit acceptIncoming();
        }
    } else {
        k->ignore();
    }
}

/*!
  \internal
  */
void CallScreen::showEvent( QShowEvent *e )
{
    if ( !m_updateTimer ) {
        m_updateTimer = new QTimer(this);
        connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updateAll()));
    }
    m_updateTimer->start(1000);
    QWidget::showEvent(e);
}

/*!
  \internal
  */
void CallScreen::keyReleaseEvent(QKeyEvent *k)
{
    if (k->key() == Qt::Key_Flip && m_control->hasIncomingCall()) {
        QSettings cfg("Trolltech","Phone");
        cfg.beginGroup("FlipFunction");
        if ( cfg.value("answer").toBool() )
            emit acceptIncoming();
    }
}

/*!
  \internal
  */
void CallScreen::closeEvent(QCloseEvent *e)
{
    if (m_listView && m_listView->selectionMode() == QAbstractItemView::SingleSelection) {
        e->ignore();
        setWindowTitle(tr("Calls"));
        setSelectMode(false);
        stateChanged();
    } else if (m_hadActiveCall) {
        setWindowState(Qt::WindowMinimized);
        e->ignore();
    } else {
        e->accept();
    }
}

/*!
  \internal
  */
void CallScreen::hideEvent( QHideEvent * )
{
    if ( m_updateTimer ) {
        m_updateTimer->stop();
    }

    updateAll(true);
}

/*!
  \internal
  */
bool CallScreen::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_listView) {
        if (e->type() == QEvent::KeyPress) {
            QKeyEvent *ke = (QKeyEvent *)e;
            if (m_listView->selectionMode() == QAbstractItemView::NoSelection) {
                if (ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down) {
                    return true;
                } else if (ke->key() == Qt::Key_Select) {
                    // gsm key select
                    if (!m_dtmfDigits.isEmpty() && m_gsmActionTimer && m_gsmActionTimer->isActive()) {
                        actionGsmSelected();
                        return true;
                    }
                    if ( m_incoming ) {
                        if(m_control->incomingCall().startTime().secsTo(QDateTime::currentDateTime()) >= 1)
                            emit acceptIncoming();
                    } else if (!ke->isAutoRepeat()) {
                        if (m_holdCount) {
                            m_control->unhold();
                        } else {
                            m_control->hold();
                        }
                    }
                    return true;
                } else if(ke->key() == Qt::Key_Back) {
                    if (m_control->hasIncomingCall()
                            && m_control->incomingCall().startTime().secsTo(QDateTime::currentDateTime()) >= 1) {
                        if ( m_actionMute->isVisible() )
                            muteRingSelected();
                        else
                            m_control->endCall();
                        return true;
                    } else if (!m_dtmfDigits.isEmpty()) {
                        clearDtmfDigits(true);
                        return true;
                    }
                }

                if (!ke->isAutoRepeat()) {
                    QString text = ke->text();
                    if ( !text.isEmpty() ) {
                        char ch = text[0].toLatin1();
                        if ( ( ch >= 48 && ch <= 57 )
                                || ch == 'p' || ch == 'P' || ch == '+' || ch == 'w'
                                || ch == 'W' || ch == '#' || ch == '*' || ch == '@' ) {
                            if( !dialNumbers( text ) ) {
                                // Show dialer
                                QtopiaServiceRequest sd("Dialer", "showDialer(QString)");
                                sd << text;
                                sd.send();
                            }
                        }
                    }
                }
            }
        } 
    }

    return false;
}

/*!
  \internal
  */
void CallScreen::setSelectMode(bool s)
{
    if (!m_listView)
        return;

    if (s) {
        m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
        QSoftMenuBar::setLabel(m_listView, Qt::Key_Select, QSoftMenuBar::Select);
        CallItemModel *m = qobject_cast<CallItemModel *>(m_listView->model());
        for( int i = m->rowCount()-1; i >= 0; i--)
        {
            CallItemEntry *item = m->callItemEntry(m->index(i));
            if (item && item->call().state() == QPhoneCall::Connected) {
                m_listView->setCurrentIndex(m->index(i));
                if ( m_activeCount == 2 )
                    break;
            }
        }
    } else {
        QSoftMenuBar::setLabel(m_listView, Qt::Key_Select, QSoftMenuBar::NoLabel);
        m_listView->setSelectionMode(QAbstractItemView::NoSelection);
        QTimer::singleShot(0, m_listView, SLOT(clearSelection()));
    }
}

/*!
  \internal
  Informs user with extra information from SIM when control event occurs.
*/
void CallScreen::simControlEvent(const QSimControlEvent &e)
{
    if ( !e.text().isEmpty() ) {
        QString title;
        switch ( e.result() ) {
            case QSimControlEvent::Allowed: title = tr( "Allowed" ); break;
            case QSimControlEvent::NotAllowed: title = tr( "Not Allowed" ); break;
            case QSimControlEvent::AllowedWithModifications: title = tr( "Number Modified" ); break;
        }
        if ( !m_simMsgBox ) {
            m_simMsgBox = QAbstractMessageBox::messageBox
                ( 0, title, e.text(), QAbstractMessageBox::Information );
            QSoftMenuBar::removeMenuFrom( m_simMsgBox, QSoftMenuBar::menuFor( m_simMsgBox ) );
        } else {
            m_simMsgBox->setTitle( title );
            m_simMsgBox->setText( e.text() );
        }
        if ( e.result() == QSimControlEvent::NotAllowed )
            m_simMsgBox->setTimeout(3000, QAbstractMessageBox::NoButton);
        QtopiaApplication::execDialog(m_simMsgBox);
    }
}

/*! \internal */
void CallScreen::muteRingSelected()
{
    m_actionMute->setVisible(false);
    emit muteRing();

    if (m_listView)
        QSoftMenuBar::setLabel(m_listView, Qt::Key_Back, "phone/reject", tr("Send Busy"));
}

/*! \internal */
void CallScreen::callConnected(const QPhoneCall &)
{
    static QHash<QString, bool>  connectedCall;
    if (connectedCall.isEmpty()) {
        connectedCall["answer"] = false;
        connectedCall["endcall"] = true;
        connectedCall["resume"] = false;
        connectedCall["sendbusy"] = false;
        connectedCall["hold"] = true;
    }

    m_view->setActiveItems(connectedCall);
}

/*! \internal */
void CallScreen::callDropped(const QPhoneCall &)
{
    static QHash<QString, bool> activeCalls;
    static QHash<QString, bool> callsOnHolds;

    if (activeCalls.isEmpty()) {
        activeCalls["hold"] = true;
        activeCalls["endcall"] = true;
        activeCalls["answer"] = false;
        activeCalls["sendbusy"] = false;

        callsOnHolds["resume"] = true;
        callsOnHolds["answer"] = false;
        callsOnHolds["endcall"] = true;
        callsOnHolds["sendbusy"] = false;
    }


    if (m_control->hasActiveCalls()) {
        m_view->setActiveItems(activeCalls);
    } else if (m_control->hasCallsOnHold()) {
        m_view->setActiveItems(callsOnHolds);
    } else {
        setItemActive("menu-box", false);
    }

    updatePowerStatus();
}

/*! \internal */
void CallScreen::callDialing(const QPhoneCall &)
{
    static QHash<QString, bool> dialing;
    if (dialing.isEmpty()) {
        dialing["keypad-box"] = false;
        dialing["menu-box"] = true;
        dialing["answer"] = false;
        dialing["endcall"] = true;
        dialing["resume"] = false;
        dialing["sendbusy"] = false;
        dialing["hold"] = true;
    }

    m_view->setActiveItems(dialing);
    updatePowerStatus();
}

/*! \internal */
void CallScreen::callIncoming(const QPhoneCall &)
{
    static QHash<QString, bool> m_incoming;
    if (m_incoming.isEmpty()) {
        m_incoming["keypad-box"] = false;
        m_incoming["menu-box"] = true;
        m_incoming["hold"] = false;
        m_incoming["endcall"] = false;
        m_incoming["resume"] = false;
        m_incoming["answer"] = true;
        m_incoming["sendbusy"] = true;
    }

    m_view->setActiveItems(m_incoming);
    updatePowerStatus();
}

/*!
  \internal
  */
void CallScreen::rejectModalDialog()
{
    // Last resort.  We shouldn't have modal dialogs in the server, but
    // just in case we need to get rid of them when a call arrives.  This
    // is a bad thing to do, but far less dangerous than missing a call.
    // XXX Known modals:
    //  - category edit dialog
    QWidgetList list = QApplication::topLevelWidgets();
    QList<QPointer<QDialog> > dlgsToDelete;

    foreach(QWidget *w, list)
        if (w->isVisible() && w->inherits("QDialog"))
            dlgsToDelete.append((QDialog*)w);

    foreach(QPointer<QDialog> d, dlgsToDelete) {
        if (!d)
            continue;

        if (d->testAttribute(Qt::WA_ShowModal)) {
            qWarning("Rejecting modal dialog: %s", d->metaObject()->className());
            d->reject();
        } else {
            qWarning("Hiding non-modal dialog: %s", d->metaObject()->className());
            d->hide();
        }
    }
}

void CallScreen::updatePowerStatus()
{

    // check if we have active calls...
    bool hasActiveCall = false;

    foreach(QPhoneCall call, m_control->allCalls()) {
        if (call.state() == QPhoneCall::Idle
            || call.state() == QPhoneCall::Incoming
            || call.state() == QPhoneCall::Dialing
            || call.state() == QPhoneCall::Alerting
            || call.state() == QPhoneCall::Connected
            || call.state() == QPhoneCall::Hold) {
            hasActiveCall = true;
            break;
        }
    }

    // Claim/Release a resource
    if (!hasActiveCall && m_hadActiveCall) {
        QDBusMessage message = QDBusMessage::createMethodCall("org.openmoko.Power", "/",
                                                              "org.openmoko.Power.Core", "RemoveRequestedResourceState");
        message << QLatin1String("cpu");
        message << QLatin1String("qpe-callscreen");
        QDBusConnection::systemBus().send(message);
    } else if (hasActiveCall && !m_hadActiveCall) {
        QDBusMessage message = QDBusMessage::createMethodCall("org.openmoko.Power", "/",
                                                              "org.openmoko.Power.Core", "RequestResourceState");
        message << QLatin1String("cpu");
        message << QLatin1String("qpe-callscreen");
        message << QLatin1String("on");
        QDBusConnection::systemBus().send(message);
    } else {
        Q_ASSERT(hasActiveCall == m_hadActiveCall);
    }

    m_hadActiveCall = hasActiveCall;
}

#include "callscreen.moc"
