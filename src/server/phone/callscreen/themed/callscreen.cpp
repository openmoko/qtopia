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

#ifdef QTOPIA_CELL
#include <QSimToolkit>
#endif

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
    DialerControl *control;
};

CallItemEntry::CallItemEntry(DialerControl *ctrl, const QPhoneCall &c, ThemeListModel* model)
    : ThemeListModelEntry(model),
            callData(c),
            control(ctrl)
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
    CallItemEntry *item = m->callItemEntry(cur);
    item->setValue( "State", tr( " (Connect)",
                "describing an action to take on a call, make sure keeping the space in the beginning" ) );
    item = m->callItemEntry(prev);
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
    QHash<QAction *, QAudioStateInfo> audioModes;
    QActionGroup *actions;
    bool audioActive;
    QtopiaIpcAdaptor *mgr;
    QAudioStateConfiguration *audioConf;
};

//-----------------------------------------------------------
/* define CallAudioHandler */

CallAudioHandler::CallAudioHandler(QAudioStateConfiguration *conf, QObject *parent)
	: QObject(parent)
{
    mgr = new QtopiaIpcAdaptor("QPE/AudioStateManager", this);
    audioConf = conf;

    audioActive = false;

    connect(conf, SIGNAL(currentStateChanged(QAudioStateInfo,QAudio::AudioCapability)),
            this, SLOT(currentStateChanged(QAudioStateInfo,QAudio::AudioCapability)));
    connect(conf, SIGNAL(availabilityChanged()),
            this, SLOT(availabilityChanged()));

    QSet<QAudioStateInfo> states = conf->states("Phone");

    actions = new QActionGroup(this);

    foreach (QAudioStateInfo state, states) {
        QAction *action = new QAction(state.displayName(), this);
        action->setVisible(false);
        action->setCheckable(true);
        actions->addAction(action);
        audioModes.insert(action, state);
    }

    connect(actions, SIGNAL(triggered(QAction*)),
            this, SLOT(actionTriggered(QAction*)));
}

CallAudioHandler::~CallAudioHandler()
{
}

void CallAudioHandler::addOptionsToMenu(QMenu *menu)
{
    foreach (QAction *action, audioModes.keys()) {
	menu->addAction(action);
    }
}

void CallAudioHandler::actionTriggered(QAction *action)
{
    if (!audioActive) {
        qWarning("CallAudioHandler::actionTriggered while audio is not active!");
        return;
    }

    if (!audioModes.contains(action)) {
        qWarning("CallAudioHandler::actionTriggered - Invalid action!");
        return;
    }

    mgr->send("setProfile(QByteArray)", audioModes[action].profile());
}

void CallAudioHandler::availabilityChanged()
{
    if (!audioActive)
        return;

    QHash<QAction *, QAudioStateInfo>::const_iterator it = audioModes.constBegin();
    while (it != audioModes.constEnd()) {
        bool vis = audioConf->isStateAvailable(audioModes[it.key()]);
        it.key()->setVisible(vis);
        ++it;
    }
}

void CallAudioHandler::currentStateChanged(const QAudioStateInfo &state, QAudio::AudioCapability)
{
    if (!audioActive)
        return;

    QHash<QAction *, QAudioStateInfo>::const_iterator it = audioModes.constBegin();
    while (it != audioModes.constEnd()) {
        if (it.value() == state) {
            it.key()->setChecked(true);
            return;
        }
        ++it;
    }
}


void CallAudioHandler::callStateChanged(bool enableAudio)
{
    if (enableAudio == audioActive)
        return;

    audioActive = enableAudio;

    if (audioActive) {
        QByteArray domain("Phone");
        int capability = static_cast<int>(QAudio::OutputOnly);
        mgr->send("setDomain(QByteArray,int)", domain, capability);

        QtopiaIpcEnvelope e("QPE/AudioVolumeManager", "setActiveDomain(QString)");
        e << QString("Phone");

        availabilityChanged();
    }
    else {
        foreach (QAction *action, actions->actions()) {
            action->setChecked(false);
            action->setVisible(false);
        }

        //TODO: This needs to be fixed  up later to send the release
        // domain message instead
        QByteArray domain("Media");
        int capability = static_cast<int>(QAudio::OutputOnly);
        mgr->send("setDomain(QByteArray,int)", domain, capability);

        QtopiaIpcEnvelope e("QPE/AudioVolumeManager", "resetActiveDomain(QString)");
        e << QString("Phone");
    }
}

//===========================================================================

class MouseControlDialog : public QDialog
{
    Q_OBJECT
public:
    MouseControlDialog( QWidget* parent = 0, Qt::WFlags fl = 0 )
        : QDialog(parent, fl), m_tid(0), m_parent(parent), m_mouseUnlocked(false)
    {
        QColor c(Qt::black);
        c.setAlpha(180);

        setAttribute(Qt::WA_SetPalette, true);

        QPalette p = palette();
        p.setBrush(QPalette::Window, c);
        setPalette(p);

        QVBoxLayout *vBox = new QVBoxLayout(this);
        QHBoxLayout *hBox = new QHBoxLayout;

        QIcon icon(":icon/select");

        QLabel *l = new QLabel(this);
        l->setPixmap(icon.pixmap(44, 44));
        hBox->addStretch();
        hBox->addWidget(l);
        hBox->addStretch();

        int height = l->sizeHint().height();

        vBox->addLayout(hBox);

        l = new QLabel(this);
        l->setWordWrap(true);
        if (Qtopia::mousePreferred()) {
            l->setText(tr("Move the slider to activate the touch screen."));
        } else {
            l->setText(tr("Press key <b>Down</b> to activate the touch screen.", "translate DOWN to name of key with down arrow"));
        }
        vBox->addWidget(l);
        height += l->sizeHint().height();

        if (Qtopia::mousePreferred()) {
            m_slider = new QSlider(Qt::Horizontal, this);
            m_slider->installEventFilter(this);
            m_slider->setRange(0, 10);
            m_slider->setPageStep(1);
            connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(sliderMoved(int)));
            vBox->addWidget(m_slider);

            height += m_slider->sizeHint().height();
        }

        QRect d = QApplication::desktop()->screenGeometry();
        int dw = d.width();
        int dh = d.height();

        height += QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing);
        height += QApplication::style()->pixelMetric(QStyle::PM_LayoutTopMargin);
        height += QApplication::style()->pixelMetric(QStyle::PM_LayoutBottomMargin);
        setGeometry(20*dw/100, (dh - height)/2, 60*dw/100, height);

        m_parent->installEventFilter(this);
    }

    static const int TIMEOUT = 2000;
signals:
    void releaseMouse();
    void grabMouse();

protected:
    void timerEvent( QTimerEvent *e )
    {
        Q_UNUSED(e)
        close();
    }

    void closeEvent( QCloseEvent *e )
    {
        // if failed to unlock the touch screen
        // hand over mouse grab to call screen
        if ( Qtopia::mousePreferred() )
            if ( m_slider->value() != m_slider->maximum() )
               emit grabMouse();
        QDialog::closeEvent( e );
    }

    void showEvent( QShowEvent *e )
    {
        m_mouseUnlocked = false;
        if (Qtopia::mousePreferred()) {
            m_slider->grabMouse();
            m_slider->setValue(0);
        }

        resetTimer();
        QDialog::showEvent( e );
    }

    void keyPressEvent( QKeyEvent *e )
    {
        if ( e->key() == Qt::Key_Down ) {
            m_mouseUnlocked = true;
            emit releaseMouse();
            close();
        }
    }

    bool eventFilter( QObject *o, QEvent *e )
    {
        if ( o == m_parent ) {
            if ( e->type() == QEvent::WindowActivate ) {
                if ( !m_mouseUnlocked )
                   emit grabMouse();
            } else if ( e->type() == QEvent::WindowDeactivate ) {
                if ( !isVisible() ) {
                    m_mouseUnlocked = false;
                    emit releaseMouse();
                }
            }
        }

        if ( Qtopia::mousePreferred() && o == m_slider ) {
            // do not allow to move slider with key press
            if ( e->type() == QEvent::KeyPress
                    || e->type() == QEvent::KeyRelease )
                return true;
        }
        return false;
    }

private slots:
    void resetTimer()
    {
        killTimer( m_tid );
        m_tid = startTimer( TIMEOUT );
    }

    void sliderMoved( int value )
    {
        if ( value == m_slider->maximum() ) {
            m_slider->releaseMouse();
            close();
        } else {
            resetTimer();
        }
    }

private:
    int m_tid;
    QSlider *m_slider;
    QWidget *m_parent;
    bool m_mouseUnlocked;
};

class CallScreenKeyboardFilter : public QtopiaKeyboardFilter
{
public:
    CallScreenKeyboardFilter() {}
    ~CallScreenKeyboardFilter() {}
protected:
    virtual bool filter(int, int, int, bool, bool)
    {
        return true;
    }
};

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
    : PhoneThemedView(parent, fl), control(ctrl), digits(0), listView(0), actionGsm(0),
    activeCount(0), holdCount(0) , keypadVisible(false), mLayout( 0 ),
    updateTimer( 0 ), gsmActionTimer(0), secondaryCallScreen(0), m_model(0), m_callAudioHandler(0),
    videoWidget(0), simMsgBox(0), showWaitDlg(false), symbolTimer(0), m_mouseCtrlDlg(0)
{
    callScreen = this;
    setObjectName(QLatin1String("calls"));

    contextMenu = QSoftMenuBar::menuFor(this);

    actionAnswer = new QAction(QIcon(":icon/phone/answer"),tr("Answer", "answer call"), this);
    connect(actionAnswer, SIGNAL(triggered()), this, SIGNAL(acceptIncoming()));
    actionAnswer->setVisible(false);
    contextMenu->addAction(actionAnswer);

    actionSendBusy = new QAction(QIcon(":icon/phone/reject"), tr("Send Busy"), this);
    connect(actionSendBusy, SIGNAL(triggered()), control, SLOT(sendBusy()));
    actionSendBusy->setVisible(false);
    contextMenu->addAction(actionSendBusy);

    actionMute = new QAction(QIcon(":icon/mute"),tr("Mute"), this);
    connect(actionMute, SIGNAL(triggered()), this, SLOT(muteRingSelected()));
    actionMute->setVisible(false);
    contextMenu->addAction(actionMute);

    actionEnd = new QAction(QIcon(":icon/phone/hangup"),tr("End"),this);
    connect(actionEnd, SIGNAL(triggered()), control, SLOT(endCall()));
    actionEnd->setVisible(false);
    contextMenu->addAction(actionEnd);

    actionEndAll = new QAction(tr("End all calls"), this);
    connect(actionEndAll, SIGNAL(triggered()), control, SLOT(endAllCalls()));
    actionEndAll->setVisible(false);
    contextMenu->addAction(actionEndAll);

    actionHold = new QAction(QIcon(":icon/phone/hold"),tr("Hold"), this);
    connect(actionHold, SIGNAL(triggered()), control, SLOT(hold()));
    actionHold->setVisible(false);
    contextMenu->addAction(actionHold);

    actionResume = new QAction(QIcon(":icon/phone/resume"),tr("Resume"), this);
    connect(actionResume, SIGNAL(triggered()), control, SLOT(unhold()));
    actionResume->setVisible(false);
    contextMenu->addAction(actionResume);

    actionMerge = new QAction(QIcon(":icon/phone/conference"),tr("Join"), this);
    connect(actionMerge, SIGNAL(triggered()), control, SLOT(join()));
    actionMerge->setVisible(false);
    contextMenu->addAction(actionMerge);

    actionSplit = new QAction(tr("Split..."), this);
    connect(actionSplit, SIGNAL(triggered()), this, SLOT(splitCall()));
    actionSplit->setVisible(false);
    contextMenu->addAction(actionSplit);

    actionTransfer = new QAction(QIcon(":icon/phone/callforwarding"),tr("Transfer"),this);
    connect(actionTransfer, SIGNAL(triggered()), control, SLOT(transfer()));
    actionTransfer->setVisible(false);
    contextMenu->addAction(actionTransfer);

    connect(control, SIGNAL(callControlRequested()), this, SLOT(showProgressDlg()));
    connect(control, SIGNAL(callControlSucceeded()), this, SLOT(hideProgressDlg()));

    m_audioConf = new QAudioStateConfiguration(this);

    if (m_audioConf->isInitialized())
        initializeAudioConf();
    else
        QObject::connect(m_audioConf, SIGNAL(configurationInitialized()),
                         this, SLOT(initializeAudioConf()));

    QObject::connect(this, SIGNAL(itemReleased(ThemeItem*)),
                     this, SLOT(themeItemClicked(ThemeItem*)));

    setWindowTitle(tr("Calls"));

    if (QApplication::desktop()->numScreens() > 1) {
        // We have a secondary screen
        secondaryCallScreen = new SecondaryCallScreen;
        secondaryCallScreen->setGeometry(QApplication::desktop()->availableGeometry(1));
    }

    QObject::connect(control,
                     SIGNAL(requestFailed(QPhoneCall,QPhoneCall::Request)),
                     this,
                     SLOT(requestFailed(QPhoneCall,QPhoneCall::Request)));

    QObject::connect(control, SIGNAL(callConnected(const QPhoneCall&)),
                     this, SLOT(callConnected(const QPhoneCall&)));

    QObject::connect(control, SIGNAL(callDropped(const QPhoneCall&)),
                     this, SLOT(callDropped(const QPhoneCall&)));

    QObject::connect(control, SIGNAL(callDialing(const QPhoneCall&)),
                     this, SLOT(callDialing(const QPhoneCall&)));

    // reject any dialogs when new call coming in
    QObject::connect(control, SIGNAL(callIncoming(const QPhoneCall&)),
                     this, SLOT(rejectModalDialog()));
    QObject::connect(control, SIGNAL(callIncoming(const QPhoneCall&)),
                     this, SLOT(callIncoming(const QPhoneCall&)));

#ifndef QT_ILLUME_LAUNCHER
    QObject::connect( VideoRingtone::instance(), SIGNAL(videoWidgetReady()),
            this, SLOT(setVideoWidget()) );
    QObject::connect( VideoRingtone::instance(), SIGNAL(videoRingtoneStopped()),
            this, SLOT(deleteVideoWidget()) );
    // delete the video widget once call is answered
    connect( this, SIGNAL(acceptIncoming()),
            this, SLOT(deleteVideoWidget()) );
#endif

#ifdef QTOPIA_CELL
    simToolkit = new QSimToolkit( QString(), this );
    QObject::connect( simToolkit, SIGNAL(controlEvent(QSimControlEvent)),
            this, SLOT(simControlEvent(QSimControlEvent)) );
#endif

#ifdef QT_ILLUME_LAUNCHER
    m_screenSaverCommand = QString::fromLocal8Bit(qgetenv("ILLUME_PHONE_CALL"));
#endif

    // Due to delayed intialization of call screen
    // manual update on incoming call is required when call screen is created
    if ( control->hasIncomingCall() )
        callIncoming( control->incomingCall() );
}

#ifndef QT_ILLUME_LAUNCHER
/*!
  Sets the video player widget to the CallScreen.
  */
void CallScreen::setVideoWidget()
{
    if ( !m_model->rowCount() )
        return;

    videoWidget = VideoRingtone::instance()->videoWidget();

    videoWidget->setParent( this );

    QRect availableGeometry = rect();
    QRect lastItemRect =
        listView->visualRect( m_model->index( m_model->rowCount() - 1 ) );

    availableGeometry.setTop(
            lastItemRect.top()
            + lastItemRect.height() );

    videoWidget->setGeometry( availableGeometry );

    // set menu.
    QMenu *menu = QSoftMenuBar::menuFor( videoWidget );
    menu = contextMenu;
    QSoftMenuBar::setLabel(videoWidget, Qt::Key_Select, "phone/answer", tr("Answer"));
    QSoftMenuBar::setLabel(listView, Qt::Key_Back, ":icon/mute", tr("Mute"));

    qLog(Media) << "Displaying the video ringtone";
    videoWidget->show();
}

/*!
  Hides the video player widget.
*/
void CallScreen::deleteVideoWidget()
{
    if (videoWidget != 0)
        delete videoWidget;

    videoWidget = 0;
}
#endif

void CallScreen::initializeAudioConf()
{
    // add speaker, bluetooth headset actions, etc
    m_callAudioHandler = new CallAudioHandler(m_audioConf, this);
    m_callAudioHandler->addOptionsToMenu(contextMenu);
}


/*!
  \internal
  */
bool CallScreen::dialNumbers(const QString & numbers)
{
    // allow to enter '+' symbol by pressing '*' key twice quickly
    // required when an internationl number is entered
    if ( numbers == "*" ) {
        if ( dtmfDigits.endsWith( "*" )
                && symbolTimer->isActive() ) {
            dtmfDigits = dtmfDigits.left( dtmfDigits.length() - 1 );
            appendDtmfDigits( "+" );
            return true;
        } else {
            if ( !symbolTimer ) {
                symbolTimer = new QTimer( this );
                symbolTimer->setSingleShot( true );
            }
            symbolTimer->start( 500 );
        }
    }

    // do not send dtmf tones while dialing for now.
    // but need a way to queue dtmf tones while dialing.
    // e.g. a phone number followed by an extension.
    if (/*control->isDialing() || */control->hasActiveCalls()) {
        // Inject the specified digits into the display area.
        control->activeCalls().first().tone(numbers);
        appendDtmfDigits(numbers);
        return true;
    } else if ( control->hasIncomingCall() ) {
        appendDtmfDigits(numbers);
    } else if ( control->hasCallsOnHold() ) {
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
    item = (ThemeListItem *)findItem( "callscreen", ThemedView::List, ThemeItem::All, false );
    delete mLayout;
    mLayout = 0;
    if( !item ) {
        qWarning("No callscreen element defined for CallScreen theme.");
        mLayout = new QVBoxLayout( this );
        listView = new CallItemListView(0,this);
    } else {
        listView = qobject_cast<CallItemListView *>(item->widget());
        Q_ASSERT(listView != 0 );
        // active item
        // hold item
        // joined call indicator
        // contact image
        // phone number text
        // status text
    }
    listView->setFrameStyle(QFrame::NoFrame);
    listView->installEventFilter(this);
    listView->setSelectionMode(QAbstractItemView::NoSelection);
    listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(listView, SIGNAL(activated(QModelIndex)), this, SLOT(callSelected(QModelIndex)));
    connect(listView, SIGNAL(clicked(QModelIndex)), this, SLOT(callClicked(QModelIndex)));
    QSoftMenuBar::setLabel(listView, Qt::Key_Select, QSoftMenuBar::NoLabel);

    item = (ThemeWidgetItem *)findItem( "callscreennumber", ThemedView::Widget, ThemeItem::All, false );
    if( !item ) {
        qWarning("No callscreennumber input element defined for CallScreen theme.");
        if( !mLayout )
            mLayout = new QVBoxLayout( this );
        digits = new QLineEdit( this );
    } else {
        digits = qobject_cast<QLineEdit*>(item->widget());
        Q_ASSERT(digits != 0);
    }
    digits->setFrame(false);
    digits->setReadOnly(true);
    digits->setFocusPolicy(Qt::NoFocus);
    digits->hide();
    connect( digits, SIGNAL(textChanged(QString)),
            this, SLOT(updateLabels()) );

    if( mLayout ) {
        mLayout->addWidget( listView );
        mLayout->addWidget( digits );
    } else {
        manualLayout();
    }

    stateChanged();
}

/*!
  \internal
  */
void CallScreen::manualLayout()
{
    QHash<QString, bool>  layout;
    layout["keypad-box"] = keypadVisible;
    layout["keypad-showcontainer"] = !keypadVisible;

    setActiveItems(layout);
}

/*!
  \internal
  */
QString CallScreen::ringTone()
{
    CallItemModel* m = qobject_cast<CallItemModel *>(listView->model());
    for (int i = m->rowCount()-1; i>=0; i--) {
        CallItemEntry* item = m->callItemEntry(m->index(i));
        if ( item && item->callData.callState == QPhoneCall::Incoming ) {
            return item->callData.ringTone;
        }
    }
    return QString();
}

/*!
  \internal
  */
void CallScreen::clearDtmfDigits(bool clearOneChar)
{
    if(dtmfDigits.isEmpty())
        return;

    if (clearOneChar)
        dtmfDigits = dtmfDigits.left(dtmfDigits.length() - 1);
    else
        dtmfDigits.clear();
    digits->setText(dtmfDigits);

    if (dtmfDigits.isEmpty()) {
        digits->hide();
        updateLabels();
    } else if (gsmActionTimer) {
        gsmActionTimer->start();
    }

    manualLayout();
    CallItemModel* m = qobject_cast<CallItemModel *>(listView->model());
    m->triggerUpdate();

    // remove menu item
    setGsmMenuItem();
}

/*!
  \internal
  */
void CallScreen::setGsmMenuItem()
{
    if (!actionGsm) {
        actionGsm = new QAction(QIcon(":icon/phone/answer"),QString(), this);
        connect(actionGsm, SIGNAL(triggered()), this, SLOT(actionGsmSelected()));
        QSoftMenuBar::menuFor(this)->addAction(actionGsm);
    }

    bool filterable = false;
    emit testKeys( dtmfDigits, filterable );

    actionGsm->setVisible(!dtmfDigits.isEmpty());

    // update menu text & lable for Key_Select
    if (!dtmfDigits.isEmpty() ) {
        if (filterable) {
            actionGsm->setText(tr("Send %1").arg(dtmfDigits));
            QSoftMenuBar::setLabel(listView, Qt::Key_Select, "", tr("Send"));
        } else {
            actionGsm->setText(tr("Call %1", "%1=phone number").arg(dtmfDigits));
            QSoftMenuBar::setLabel(listView, Qt::Key_Select, "phone/answer", tr("Call"));
        }
    }
}

/*!
  \internal
  */
void CallScreen::actionGsmSelected()
{
    bool filtered = false;
    emit filterSelect(dtmfDigits, filtered);
    // if the digits are not filtered place a call
    if ( !filtered ) {
        // check if contact exists
        QContactModel *m = ServerContactModel::instance();
        QContact cnt = m->matchPhoneNumber(dtmfDigits);

        if ( cnt == QContact() ) { // no contact
            QtopiaServiceRequest service( "Dialer", "dial(QString,QString)" );
            service << QString() << dtmfDigits;
            service.send();
        } else {
            QtopiaServiceRequest service( "Dialer", "dial(QString,QUniqueId)" );
            service << dtmfDigits << cnt.uid();
            service.send();
        }
    }
    // clear digits wheather filtered or not
    clearDtmfDigits();
}

/*!
  \internal
  */
void CallScreen::updateLabels()
{
    // update context label according to the current call count.
    if (control->allCalls().count() >= 2)
        QSoftMenuBar::setLabel(listView, Qt::Key_Select, "phone/swap", tr("Swap"));
    else if (control->activeCalls().count() == 1)
        QSoftMenuBar::setLabel(listView, Qt::Key_Select, "phone/hold", tr("Hold"));
    else
        QSoftMenuBar::setLabel(listView, Qt::Key_Select, QSoftMenuBar::NoLabel);

    // display clear icon when dtmf digits are entered.
    if (digits->text().isEmpty())
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Back);
    else
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace);
}

/*!
  \internal
  */
void CallScreen::appendDtmfDigits(const QString &dtmf)
{
    dtmfDigits.append(dtmf);
    if(dtmfDigits.isEmpty())
        return;

    digits->setText(dtmfDigits);
    digits->setCursorPosition(digits->text().length());
    digits->show();

    // if video widget is shown reduce the size
    if ( videoWidget ) {

        QRect curGeometry = videoWidget->geometry();
        QRect digitsRect = digits->geometry();

        curGeometry.setBottom( digitsRect.top() );

        videoWidget->setGeometry( curGeometry );
    }

    manualLayout();
    CallItemModel* m = qobject_cast<CallItemModel *>(listView->model());
    m->triggerUpdate();

    // add menu item.
    setGsmMenuItem();

    if (!gsmActionTimer) {
        gsmActionTimer = new QTimer(this);
        gsmActionTimer->setInterval(SELECT_KEY_TIMEOUT);
        gsmActionTimer->setSingleShot(true);
        QObject::connect(gsmActionTimer, SIGNAL(timeout()), this, SLOT(updateLabels()));
    }
    gsmActionTimer->start();

    // filter immediate action
    bool filtered = false;
    emit filterKeys( dtmfDigits, filtered );
    if ( filtered ) {
        clearDtmfDigits();
    }
}

/*!
  \internal
  */
void CallScreen::stateChanged()
{
    if( !listView || !digits )
        return;
    const QList<QPhoneCall> &calls = control->allCalls();


    // see if any calls have ended.

    CallItemEntry *item = 0;
    CallItemModel *m = qobject_cast<CallItemModel *>(listView->model());
    for (int i = m->rowCount()-1; i>=0; i--) {
        item = m->callItemEntry(m->index(i));
        if (item && !calls.contains(item->call())) {
            if (item->callData.disconnectTime.isNull()) {
                item->callData.disconnectTime = QDateTime::currentDateTime();
                item->setText("f");
            }
        }
    }

    activeCount = 0;
    holdCount = 0;
    incoming = false;
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

            item = new CallItemEntry(control, call, m);
            m->addEntry(item);
            manualLayout();
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
            activeCount++;
            if (!active)
                active = item;
            sortOrder = 2;
            state = CallScreen::tr("Connected", "call state");
            if ( simMsgBox && simMsgBox->isVisible() )
                simMsgBox->hide();
        } else if (call.state() == QPhoneCall::Hold) {
            holdCount++;
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
            incoming = true;
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

#if QT_ILLUME_LAUNCHER
       informScreenSaver(item, name); 
#endif
    }

    for (int i = m->rowCount()-1; i>=0; i--) {
        if (!m->index(i).isValid())
            break;
        CallItemEntry* item = m->callItemEntry(m->index(i));
        if( !item )
            continue;
        if( item->call().dropped() ) {
            item->setValue("State", tr( " (Disconnected)", "describing call state, make sure keeping the space in the beginning") );

#if QT_ILLUME_LAUNCHER
           informScreenSaver(item, item->callData.numberOrName.isEmpty() ? tr("Unknown caller") : item->callData.numberOrName); 
#endif
        }
    }

    if (secondaryCallScreen && primaryItem)
        secondaryCallScreen->setCallData(primaryItem->callData);

    // update available actions.
    actionAnswer->setVisible(control->hasIncomingCall());
    actionSendBusy->setVisible(control->hasIncomingCall());
    actionMute->setVisible(control->hasIncomingCall());
    actionHold->setVisible(activeCount && !holdCount && !incoming && !dialing);
    actionResume->setVisible(holdCount && !incoming && !dialing);
    actionEnd->setVisible((activeCount || holdCount || dialing) && !incoming);
    actionEndAll->setVisible(activeCount && holdCount && !incoming);
    actionMerge->setVisible(activeCount && holdCount &&
                            activeCount < MAX_JOINED_CALLS &&
                            holdCount < MAX_JOINED_CALLS && !incoming);
    actionSplit->setVisible(activeCount > 1 && !holdCount && !incoming);
    actionTransfer->setVisible(activeCount == 1 && holdCount == 1 && !incoming);

    // update the speaker and bluetooth headset actions.
    bool nonActiveDialing = dialing && !activeCount;

    if (m_callAudioHandler)
        m_callAudioHandler->callStateChanged(activeCount || holdCount || nonActiveDialing /* || incoming*/ );

    if (incoming) {
        QSoftMenuBar::setLabel(listView, Qt::Key_Select, "phone/answer", tr("Answer"));
    } else if (activeCount && holdCount) {
        actionResume->setText(tr("Swap", "change to 2nd open phoneline and put 1st on hold"));
        actionResume->setIcon(QPixmap(":icon/phone/swap"));
        QSoftMenuBar::setLabel(listView, Qt::Key_Select, "phone/swap", tr("Swap"));
    } else if (holdCount && !activeCount && !dialing && !incoming) {
        actionResume->setText(tr("Resume"));
        actionResume->setIcon(QIcon(":icon/phone/resume"));
        QSoftMenuBar::setLabel(listView, Qt::Key_Select, "phone/resume", tr("Resume"));
    } else if (activeCount && !holdCount && !dialing && !incoming) {
        QSoftMenuBar::setLabel(listView, Qt::Key_Select, "phone/hold", tr("Hold"));
    } else {
        QSoftMenuBar::setLabel(listView, Qt::Key_Select, QSoftMenuBar::NoLabel);
    }

    if (incoming && listView->selectionMode() != QAbstractItemView::SingleSelection)
        if ( incoming )
            QSoftMenuBar::setLabel(listView, Qt::Key_Back, ":icon/mute", tr("Mute"));
        else
            QSoftMenuBar::setLabel(listView, Qt::Key_Back, QSoftMenuBar::NoLabel);
    else
        QSoftMenuBar::clearLabel(listView, Qt::Key_Back);

    //layout()->activate();
    m->sort(0, Qt::AscendingOrder);
    if (m->rowCount() > 0)
        listView->scrollTo(m->index(0));

    if( itemStateChanged ) // any items state changed?
        updateAll();
    else
        m->triggerUpdate();

    // Check dtmf status
    if(!active || active->call().identifier() != dtmfActiveCall)
        clearDtmfDigits();
    if(active)
        dtmfActiveCall = active->call().identifier();
    else
        dtmfActiveCall = QString();
    update();
}

/*!
  \internal
  */
void CallScreen::requestFailed(const QPhoneCall &,QPhoneCall::Request r)
{
    hideProgressDlg();

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
    if( !listView || !digits )
        return;

    CallItemModel *m = qobject_cast<CallItemModel *>(listView->model());
    for (int i = m->rowCount()-1; i>=0; i--) {
        if (!m->index(i).isValid())
            break;
        CallItemEntry* item = m->callItemEntry(m->index(i));
        if (item->call().state() == QPhoneCall::ServiceHangup) {
            // USSD message is coming soon, so remove from the call screen.
            m->removeEntry(m->index(i)); // removeEntry will delete the item
            manualLayout();
            i--;
            continue;
        } else if (item->call().dropped()) {
            // Remove dropped calls after a short delay
            if (!item->callData.disconnectTime.isNull() &&
                item->callData.disconnectTime.time().elapsed() > 3000) {
                m->removeEntry(m->index(i)); // removeEntry will delete the item
                manualLayout();
                i--;
                continue;
            }
        }

        /* Set appropriate information in the CallItemModel item */
        item->setValue( "Duration", item->callData.durationString() );
        if( item->value("Photo").toString().isEmpty() )
            item->setValue( "Photo", item->callData.photo );
        if (secondaryCallScreen && item->call() == secondaryCallScreen->call()) {
            secondaryCallScreen->setCallData(item->callData);
            secondaryCallScreen->showMaximized();
        }
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
    actionHold->setVisible(false);
    actionResume->setVisible(false);
    actionEnd->setVisible(false);
    actionEndAll->setVisible(false);
    actionMerge->setVisible(false);
    actionSplit->setVisible(false);
    actionTransfer->setVisible(false);
}

/*!
  \internal
  */
void CallScreen::callSelected(const QModelIndex& index)
{
    CallItemModel* m = qobject_cast<CallItemModel *>(listView->model());

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
                control->unhold();
            else if (callItem->call().connected())
                control->hold();
        }
    }
}

/*!
  \internal
  */
void CallScreen::callClicked(const QModelIndex& index)
{
/*  Comment this out for the moment. We don't want to click on call items even if the touchscreen
    is the preferred method of input. We might want to put it back for a desk phone. */
    Q_UNUSED(index)
/*
    CallItemModel* m = qobject_cast<CallItemModel *>(listView->model());
    CallItemEntry *callItem = m->callItemEntry(index);
    if (!callItem)
        return;

    if (listView->selectionMode() == QAbstractItemView::NoSelection) {
        // Only perform these operations if touchscreen is the preferred method of input.
        //    This stops the user's ear putting a call on hold or accepting an incoming call etc.
        if(Qtopia::mousePreferred()) {
            // We are not in selection mode.
            if (incoming && callItem->call().incoming()) {
                if(control->incomingCall().startTime().secsTo(QDateTime::currentDateTime()) >= 1)
                    emit acceptIncoming();
            } else if (callItem->call().onHold()) {
                control->unhold();
            } else if (callItem->call().connected()) {
                control->hold();
            }
        }
    } else {
        // In selection mode - just used to split calls at the moment
        if (m->flags(index) & Qt::ItemIsSelectable) {
            setSelectMode(false);
            //XXX I could be used for more than just split
            callItem->call().activate(QPhoneCall::CallOnly);
            setWindowTitle(tr("Calls"));
        }
    }
*/
}

/*!
  \internal
  */
void CallScreen::setItemActive(const QString &name, bool active)
{
    ThemeItem *item = (ThemeItem *)findItem(name, ThemedView::Item, ThemeItem::All, false);
    if (item)
        item->setActive(active);
}

/*!
  \internal
  */
void CallScreen::themeItemClicked(ThemeItem *item)
{
    if (!item)
        return;

    // if the touch screen is locked to nothing
    if (QWidget::mouseGrabber() == this)
        return;

    if (item->itemName() == "answer")
    {
        actionAnswer->trigger();
    }
    else if (item->itemName() == "endcall")
    {
        actionEnd->trigger();
    }
    else if (item->itemName() == "hold")
    {
        if (!control->hasActiveCalls())
            return;

        // FIXME, TODO, bad assumption, hold may fail?
        item->setActive(false);
        setItemActive("resume", true);
        actionHold->trigger();
    }
    else if (item->itemName() == "resume")
    {
        if (!control->hasCallsOnHold())
            return;

        // FIXME, TODO, bad assumption, resume may fail?
        item->setActive(false);
        setItemActive("hold", true);
        actionResume->trigger();
    }
    else if (item->itemName() == "sendbusy")
    {
        actionSendBusy->trigger();
    }
    else if (item->itemName() == "show_keypad")
    {
        static QHash<QString, bool> showKeypad;
        if (showKeypad.isEmpty()) {
            showKeypad["menu-box"]   = false;
            showKeypad["keypad-box"] = true;
        }

        setActiveItems(showKeypad);
    }
    else if (item->itemName() == "hide_keypad")
    {
        static QHash<QString, bool> hideKeypad;
        if (hideKeypad.isEmpty()) {
            hideKeypad["menu-box"]   = true;
            hideKeypad["keypad-box"] = false;
        }

        setActiveItems(hideKeypad);
    }
    else if (item->itemName().left( 11 ) == "keypad-show") {
        // themed touchscreen keypad
        keypadVisible = true;
        manualLayout();
    }
    else if ( item->itemName().left( 11 ) == "keypad-hide" )
    {
        keypadVisible = false;
        manualLayout();
        clearDtmfDigits();
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
            control->endAllCalls();
            hide();
        }
    } else if (k->key() == Qt::Key_Hangup || k->key() == Qt::Key_No) {
        if (control->isConnected() || control->isDialing() || control->hasIncomingCall())
            control->endCall();
        else
            hide();
    } else if ((k->key() == Qt::Key_F28) && control->isConnected() && !control->hasIncomingCall()) {
        control->endCall();
    } else if (k->key() == Qt::Key_Call || k->key() == Qt::Key_Yes || k->key() == Qt::Key_F28) {
        if (!dtmfDigits.isEmpty()) {
            actionGsmSelected();
        } else {
            if ( control->hasIncomingCall() )
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
    if ( !updateTimer ) {
        updateTimer = new QTimer(this);
        connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateAll()));
    }
    updateTimer->start(1000);
    ThemedView::showEvent( e );
    manualLayout();
    QTimer::singleShot(0, this, SLOT(initializeMouseControlDialog()));
}

/*!
  \internal
  */
void CallScreen::keyReleaseEvent(QKeyEvent *k)
{
    if (k->key() == Qt::Key_Flip && control->hasIncomingCall()) {
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
    if (listView && listView->selectionMode() == QAbstractItemView::SingleSelection) {
        e->ignore();
        setWindowTitle(tr("Calls"));
        setSelectMode(false);
        stateChanged();
    } else if (control->hasIncomingCall()) {
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
    if ( updateTimer )
        updateTimer->stop();

    if (secondaryCallScreen)
        secondaryCallScreen->hide();
}

/*!
  \internal
  */
bool CallScreen::eventFilter(QObject *o, QEvent *e)
{
    if (o == listView) {
        if (e->type() == QEvent::KeyPress) {
            QKeyEvent *ke = (QKeyEvent *)e;
            if (listView->selectionMode() == QAbstractItemView::NoSelection) {
                if (ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down) {
                    return true;
                } else if (ke->key() == Qt::Key_Select) {
                    // gsm key select
                    if (!dtmfDigits.isEmpty() && gsmActionTimer && gsmActionTimer->isActive()) {
                        actionGsmSelected();
                        return true;
                    }
                    if ( incoming ) {
                        if(control->incomingCall().startTime().secsTo(QDateTime::currentDateTime()) >= 1)
                            emit acceptIncoming();
                    } else if (!ke->isAutoRepeat()) {
                        if (holdCount) {
                            control->unhold();
                        } else {
                            control->hold();
                        }
                    }
                    return true;
                } else if(ke->key() == Qt::Key_Back) {
                    if (control->hasIncomingCall()
                            && control->incomingCall().startTime().secsTo(QDateTime::currentDateTime()) >= 1) {
                        if ( actionMute->isVisible() )
                            muteRingSelected();
                        else
                            control->endCall();
                        return true;
                    } else if (!dtmfDigits.isEmpty()) {
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
        } else if (e->type() == QEvent::Show) {
            grabMouse();
        }
    }

    return false;
}

/*!
  \internal
  */
void CallScreen::setSelectMode(bool s)
{
    if (s) {
        listView->setSelectionMode(QAbstractItemView::SingleSelection);
        QSoftMenuBar::setLabel(listView, Qt::Key_Select, QSoftMenuBar::Select);
        CallItemModel *m = qobject_cast<CallItemModel *>(listView->model());
        for( int i = m->rowCount()-1; i >= 0; i--)
        {
            CallItemEntry *item = m->callItemEntry(m->index(i));
            if (item && item->call().state() == QPhoneCall::Connected) {
                listView->setCurrentIndex(m->index(i));
                if ( activeCount == 2 )
                    break;
            }
        }
    } else {
        QSoftMenuBar::setLabel(listView, Qt::Key_Select, QSoftMenuBar::NoLabel);
        listView->setSelectionMode(QAbstractItemView::NoSelection);
        QTimer::singleShot(0, listView, SLOT(clearSelection()));
    }
}

/* Reimplemented from ThemedView */
/*!
  \internal
  */
QWidget *CallScreen::newWidget(ThemeWidgetItem* input, const QString& name)
{
    if( name == "callscreen" )  {
        Q_ASSERT(input->rtti() == ThemedView::List);
        CallItemListView * lv = new CallItemListView( input, this );
        if(m_model != 0)
            delete m_model;
        m_model = new CallItemModel( this, static_cast<ThemeListItem*>(input), this );
        lv->setModel(m_model);
        return lv;
    } else if( name == "callscreennumber" ) {
        return new QLineEdit( this );
    }
    return 0;
}

#ifdef QTOPIA_CELL

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
        if ( !simMsgBox ) {
            simMsgBox = QAbstractMessageBox::messageBox
                ( 0, title, e.text(), QAbstractMessageBox::Information );
            QSoftMenuBar::removeMenuFrom( simMsgBox, QSoftMenuBar::menuFor( simMsgBox ) );
        } else {
            simMsgBox->setTitle( title );
            simMsgBox->setText( e.text() );
        }
        if ( e.result() == QSimControlEvent::NotAllowed )
            simMsgBox->setTimeout(3000, QAbstractMessageBox::NoButton);
        QtopiaApplication::execDialog(simMsgBox);
    }
}

#endif

/*! \internal */
void CallScreen::grabMouse()
{
    // lock touch screen
    if (!Qtopia::mousePreferred())
        PhoneThemedView::grabMouse();
}

/*! \internal */
void CallScreen::releaseMouse()
{
    // unlock touch screen
    if (!Qtopia::mousePreferred())
        PhoneThemedView::releaseMouse();
}

void CallScreen::initializeMouseControlDialog()
{
    // Do not use screen lock if touch screen only
    if (Qtopia::mousePreferred())
        return;

    if ( !m_mouseCtrlDlg ) {
        m_mouseCtrlDlg = new MouseControlDialog(this, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        connect( m_mouseCtrlDlg, SIGNAL(releaseMouse()), this, SLOT(releaseMouse()) );
        connect( m_mouseCtrlDlg, SIGNAL(grabMouse()), this, SLOT(grabMouse()) );
    }
}

/*! \internal */
void CallScreen::mousePressEvent(QMouseEvent *e)
{
    // if touch screen is not locked no need to show unlock dialog
    if ( !QWidget::mouseGrabber() ) {
        PhoneThemedView::mousePressEvent(e);
        return;
    }

    // if touch screen only phone, the mouse control dialog need to grab mouse
    // so release the mouse before show the dialog.
    if ( Qtopia::mousePreferred() )
        releaseMouse();

    if ( m_mouseCtrlDlg )
        m_mouseCtrlDlg->show();
}

/*! \internal */
void CallScreen::muteRingSelected()
{
    actionMute->setVisible(false);
    emit muteRing();
    QSoftMenuBar::setLabel(listView, Qt::Key_Back, "phone/reject", tr("Send Busy"));
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

    setActiveItems(connectedCall);
}

/*! \internal */
void CallScreen::callDropped(const QPhoneCall &)
{
    static QHash<QString, bool> activeCalls;
    static QHash<QString, bool> callsOnHolds;

    if (activeCalls.isEmpty()) {
        activeCalls["hold"] = true;
        activeCalls["endcall"] = true;

        callsOnHolds["resume"] = true;
        callsOnHolds["answer"] = true;
    }


    if (control->hasActiveCalls()) {
        setActiveItems(activeCalls);
    } else if (control->hasCallsOnHold()) {
        setActiveItems(callsOnHolds);
    } else {
        setItemActive("menu-box", false);
    }

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

    setActiveItems(dialing);
}

/*! \internal */
void CallScreen::callIncoming(const QPhoneCall &)
{
    QtopiaInputEvents::addKeyboardFilter( new CallScreenKeyboardFilter );

    static QHash<QString, bool> incoming;
    if (incoming.isEmpty()) {
        incoming["keypad-box"] = false;
        incoming["menu-box"] = true;
        incoming["hold"] = false;
        incoming["endcall"] = false;
        incoming["resume"] = false;
        incoming["answer"] = true;
        incoming["sendbusy"] = true;
    }

    setActiveItems(incoming);
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

/*!
  \internal
*/
void CallScreen::interactionDelayTimeout()
{
    QtopiaInputEvents::removeKeyboardFilter();

    stateChanged();
}

/*! \internal */
void CallScreen::showProgressDlg()
{
}

/*! \internal */
void CallScreen::hideProgressDlg()
{
}

#ifdef QT_ILLUME_LAUNCHER
/*! \internal */
void CallScreen::informScreenSaver(CallItemEntry* entry, const QString& name)
{
    if (m_screenSaverCommand.isEmpty())
        return;

    QString state = "unknown"; // No tr
    const QPhoneCall& call = entry->callData.call;
    if (call.dropped())
        state = "disconnected";
    else {
        switch (call.state()) {
        case QPhoneCall::Connected:
            state = "connected";
            break;
        case QPhoneCall::Hold:
            state = "hold";
            break;
        case QPhoneCall::Dialing: // fall through
        case QPhoneCall::Alerting:
            state = "dialing";
            break;
        case QPhoneCall::Incoming:
            state = "incoming";
            break;
        case QPhoneCall::Idle: // fall through
        case QPhoneCall::HangupLocal: // fall through
        case QPhoneCall::HangupRemote: // fall through
        case QPhoneCall::Missed: // fall through
        case QPhoneCall::NetworkFailure: // fall through
        case QPhoneCall::OtherFailure: // fall through
        case QPhoneCall::ServiceHangup: // fall through
            break;

        }
    }

    QStringList parameters;
    parameters << call.number() << name << state;
    if (entry->callData.havePhoto)
       parameters << entry->callData.photo; 
    QProcess::startDetached(m_screenSaverCommand, parameters);
}
#endif

#include "callscreen.moc"
