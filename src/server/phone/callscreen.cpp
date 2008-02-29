/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "callscreen.h"
#include "dialercontrol.h"
#include "homescreen.h"
#include "servercontactmodel.h"
#include "qabstractmessagebox.h"
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
#include <QSqlQuery>
#include <QList>

#include <qexportedbackground.h>
#include <qtopiaservices.h>
#include <qsoftmenubar.h>
#include <qtopia/pim/qcontactmodel.h>
#include <qtopia/pim/qcontact.h>
#include <qtopianamespace.h>
#include <themedview.h>
#include <qspeakerphoneaccessory.h>

#ifdef QTOPIA_BLUETOOTH
#include <QBluetoothAudioGateway>
#endif


static const int  MAX_JOINED_CALLS = 5;
static const uint SECS_PER_HOUR= 3600;
static const uint SECS_PER_MIN  = 60;

#define SELECT_KEY_TIMEOUT 2000

static CallScreen *callScreen = 0;

class CallData {
public:
    CallData() {}
    CallData(const QPhoneCall &c) : call(c), havePhoto(false) {
        // Get the number or name to display in the text area.
        numberOrName = call.number();

        QContact cnt;
        QContactModel *m = ServerContactModel::instance();
        if (!call.contact().isNull()) {
            cnt = m->contact(call.contact());
#ifdef QTOPIA_VOIP
        } else if (numberOrName.contains(QChar('@'))) {
            QSqlQuery q;
            q.prepare("SELECT recid FROM contactcustom WHERE fieldname = :fn AND (fieldvalue = :fv OR fieldvalue = :sv)");
            q.bindValue(":fn", "VOIP_ID");
            q.bindValue(":fv", numberOrName);
            q.bindValue(":sv", "sip:" + numberOrName);
            q.exec();
            if(q.next())
                cnt = m->contact(QUniqueId(q.value(0).toByteArray()));
#endif
        } else if (!numberOrName.isEmpty()) {
            QString name;
            cnt = m->matchPhoneNumber(numberOrName);
        }

        if (!cnt.uid().isNull()) {
            numberOrName = cnt.label();
            ringTone = cnt.customField( "tone" );
            QString pf = cnt.portraitFile();
            if( pf.isEmpty() ) {
                photo = ":image/addressbook/generic-contact";
                havePhoto = false;
            } else {
                photo = Qtopia::applicationFileName( "addressbook", "contactimages/" ) + cnt.portraitFile();
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
public:
    CallItemListView(ThemeWidgetItem *ti, QWidget *parent=0);
protected:
    void keyPressEvent(QKeyEvent*);
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

//===========================================================================

/* declare CallAudioHandler */
class CallAudioHandler : public QObject
{
	Q_OBJECT
public:
	CallAudioHandler(QObject *parent = 0);
	~CallAudioHandler();

	void addOptionsToMenu(QMenu *menu);
	void callStateChanged(bool enableAudio);
	
	void setSpeakerEnabled(bool enabled);
	void setBluetoothHeadsetEnabled(bool enabled);
	
private slots:
	void triggerSpeaker();
#ifdef QTOPIA_BLUETOOTH
	void triggerBluetoothHeadset();
	void bluetoothAudioStateChanged();
	void headsetDisconnected();
#endif
	
private:
#ifdef QTOPIA_BLUETOOTH
	bool resetCurrAudioGateway();
	QList<QBluetoothAudioGateway *> *m_audioGateways;
	QBluetoothAudioGateway *m_currAudioGateway;
	QAction *actionBtHeadset;
#endif
	
	QAction *actionSpeaker;
};

//-----------------------------------------------------------
/* define CallAudioHandler */

CallAudioHandler::CallAudioHandler(QObject *parent)
	: QObject(parent)
{
	// setup speaker 
	actionSpeaker = new QAction(tr("Speaker Phone"),this);
	connect(actionSpeaker, SIGNAL(triggered()), SLOT(triggerSpeaker()));
	actionSpeaker->setVisible(false);
	actionSpeaker->setCheckable(true);
	
	// setup headset 
#ifdef QTOPIA_BLUETOOTH
	m_audioGateways = 0;
	m_currAudioGateway = 0;
	actionBtHeadset = new QAction(tr("Bluetooth Headset"),this);
    connect(actionBtHeadset, SIGNAL(triggered()), SLOT(triggerBluetoothHeadset()));
	actionBtHeadset->setVisible(false);
	actionBtHeadset->setCheckable(true);

	m_audioGateways = new QList<QBluetoothAudioGateway *>;
	m_audioGateways->append(new QBluetoothAudioGateway("BluetoothHandsfree"));
	m_audioGateways->append(new QBluetoothAudioGateway("BluetoothHeadset"));

	for (int i=0; i<m_audioGateways->size(); i++) {
		QBluetoothAudioGateway *gateway = m_audioGateways->at(i);
		connect(gateway, SIGNAL(audioStateChanged()), SLOT(bluetoothAudioStateChanged()));
		connect(gateway, SIGNAL(headsetDisconnected()), SLOT(headsetDisconnected()));
	}
#endif	
}

CallAudioHandler::~CallAudioHandler()
{
#ifdef QTOPIA_BLUETOOTH
    delete m_audioGateways;
#endif
}

void CallAudioHandler::addOptionsToMenu(QMenu *menu)
{
	menu->addAction(actionSpeaker);
#ifdef QTOPIA_BLUETOOTH
	menu->addAction(actionBtHeadset);
#endif
}

void CallAudioHandler::callStateChanged(bool enableAudio)
{
	QSpeakerPhoneAccessory speakerPhone;

	// determine if we need to show the speaker phone option.
	if (speakerPhone.available()) {
		if (enableAudio) {
			actionSpeaker->setChecked(speakerPhone.onSpeaker());
			actionSpeaker->setVisible(true);
		} else {
            speakerPhone.setOnSpeaker(false);
            actionSpeaker->setChecked(false);
            actionSpeaker->setVisible(false);
		}
	} else {
		// speaker phone support is not available.
		actionSpeaker->setVisible(false);
	}

#ifdef QTOPIA_BLUETOOTH		
		// determine if we need to show the bluetooth headset option.
		if (m_currAudioGateway || resetCurrAudioGateway()) {
			if (enableAudio) {
				if (!m_currAudioGateway->audioEnabled() && !speakerPhone.onSpeaker()) {
					m_currAudioGateway->connectAudio();
				}	
				actionBtHeadset->setChecked(m_currAudioGateway->audioEnabled());
				actionBtHeadset->setVisible(true);
			} else {
				m_currAudioGateway->releaseAudio();
				actionBtHeadset->setChecked(false);
				actionBtHeadset->setVisible(false);
			}
		} else {
        	// bluetooth headset is not available (no headsets are connected)
			actionBtHeadset->setVisible(false);	
		}
#endif
}

void CallAudioHandler::triggerSpeaker()
{
/*
	QSpeakerPhoneAccessory speakerPhone;
	bool state = !speakerPhone.onSpeaker();
	speakerPhone.setOnSpeaker(state);
	actionSpeaker->setChecked(state);
*/

	QSpeakerPhoneAccessory speakerPhone;
	if (speakerPhone.available())
		setSpeakerEnabled(!speakerPhone.onSpeaker());
}

void CallAudioHandler::setSpeakerEnabled(bool enabled)
{
	if (enabled) {
        // if turning speaker on, turn bluetooth headset off
		setBluetoothHeadsetEnabled(false);
	}

	QSpeakerPhoneAccessory speakerPhone;
	if (speakerPhone.available()) {
		speakerPhone.setOnSpeaker(enabled);
		actionSpeaker->setChecked(enabled);
	}
}

#ifdef QTOPIA_BLUETOOTH
void CallAudioHandler::triggerBluetoothHeadset()
{
	if (m_currAudioGateway || resetCurrAudioGateway())
		setBluetoothHeadsetEnabled(!m_currAudioGateway->audioEnabled());
}
#endif

void CallAudioHandler::setBluetoothHeadsetEnabled(bool enabled)
{
#ifdef QTOPIA_BLUETOOTH
	if (m_currAudioGateway || resetCurrAudioGateway()) {
		if (enabled) {
			setSpeakerEnabled(false);   // if turning headset on, turn speaker off
			m_currAudioGateway->connectAudio();
		} else {
			m_currAudioGateway->releaseAudio();
		}

		actionBtHeadset->setChecked(enabled);
	} else {
		actionBtHeadset->setChecked(false);
	}
#endif
}

#ifdef QTOPIA_BLUETOOTH
bool CallAudioHandler::resetCurrAudioGateway()
{
	for (int i=0; i<m_audioGateways->size(); i++) {
		QBluetoothAudioGateway *gateway = m_audioGateways->at(i);
		if (gateway->isConnected()) {
			m_currAudioGateway = gateway;
			return true;
		}
	}	
	
	return false;
}
#endif

#ifdef QTOPIA_BLUETOOTH
void CallAudioHandler::bluetoothAudioStateChanged()
{
	if (m_currAudioGateway || resetCurrAudioGateway()) {
		bool usingHeadset = m_currAudioGateway->audioEnabled();
		actionBtHeadset->setChecked(usingHeadset);

		// in case the audio was turned on by someone other
		// than this class, ensure the speaker is turned off
		if (usingHeadset) 
			setSpeakerEnabled(false);
	}
}
#endif

#ifdef QTOPIA_BLUETOOTH
void CallAudioHandler::headsetDisconnected()
{
	m_currAudioGateway = 0;
	actionBtHeadset->setChecked(false);
	actionBtHeadset->setVisible(false);
}
#endif

//===========================================================================

/* define CallScreen */
CallScreen::CallScreen(DialerControl *ctrl, QWidget *parent, Qt::WFlags fl)
    : PhoneThemedView(parent, fl), control(ctrl), digits(0), listView(0), actionGsm(0),
    activeCount(0), holdCount(0) , keypadVisible(false), mLayout( 0 ),
    updateTimer( 0 ), gsmActionTimer(0), secondaryCallScreen(0), m_model(0)
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
    connect(actionMute, SIGNAL(triggered()), this, SIGNAL(muteRing()));
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

    actionSplit = new QAction(tr("Split"), this);
    connect(actionSplit, SIGNAL(triggered()), this, SLOT(splitCall()));
    actionSplit->setVisible(false);
    contextMenu->addAction(actionSplit);

    actionTransfer = new QAction(QIcon(":icon/phone/callforwarding"),tr("Transfer"),this);
    connect(actionTransfer, SIGNAL(triggered()), control, SLOT(transfer()));
    actionTransfer->setVisible(false);
    contextMenu->addAction(actionTransfer);

	// add speaker, bluetooth headset actions
	m_callAudioHandler = new CallAudioHandler(this);
	m_callAudioHandler->addOptionsToMenu(contextMenu);

    QObject::connect(this, SIGNAL(itemClicked(ThemeItem*)),
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
}

bool CallScreen::dialNumbers(const QString & numbers)
{
    // do not send dtmf tones while dialing for now.
    // but need a way to queue dtmf tones while dialing.
    // e.g. a phone number followed by an extension.
    if (/*control->isDialing() || */control->hasActiveCalls()) {
        // Inject the specified digits into the display area.
        control->activeCalls().first().tone(numbers);
        appendDtmfDigits(numbers);
        return true;
    }
    return false;
}

void CallScreen::themeLoaded( const QString & )
{
    ThemeWidgetItem *item = 0;
    item = (ThemeListItem *)findItem( "callscreen", ThemedView::List );
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
    connect(listView, SIGNAL(activated(const QModelIndex&)), this, SLOT(callSelected(const QModelIndex&)));
    connect(listView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(callClicked(const QModelIndex&)));
    QSoftMenuBar::setLabel(listView, Qt::Key_Select, QSoftMenuBar::NoLabel);

    item = (ThemeWidgetItem *)findItem( "callscreennumber", ThemedView::Widget );
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

    if( mLayout ) {
        mLayout->addWidget( listView );
        mLayout->addWidget( digits );
    } else {
        manualLayout();
    }

    stateChanged();
}

void CallScreen::manualLayout()
{
    ThemeRectItem *keypaditem = (ThemeRectItem *)findItem( "keypad-box", ThemedView::Rect );
    ThemeRectItem *keypadbutton = (ThemeRectItem *)findItem( "keypad-show-container", ThemedView::Rect );
    if( keypaditem && keypadbutton ) {
        keypaditem->setActive( keypadVisible );
        keypadbutton->setActive( !keypadVisible );

    }
    update();
}

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

void CallScreen::actionGsmSelected()
{
    bool filtered = false;
    emit filterSelect(dtmfDigits, filtered);
    // clear digits wheather filtered or not 
    clearDtmfDigits();
}

void CallScreen::updateLabels()
{
    // update context label according to the current call count.
    if (control->allCalls().count() >= 2)
        QSoftMenuBar::setLabel(listView, Qt::Key_Select, "phone/swap", tr("Swap"));
    else if (control->activeCalls().count() == 1)
        QSoftMenuBar::setLabel(listView, Qt::Key_Select, "phone/hold", tr("Hold"));
    else
        QSoftMenuBar::setLabel(listView, Qt::Key_Select, QSoftMenuBar::NoLabel);
} 

void CallScreen::appendDtmfDigits(const QString &dtmf)
{
    dtmfDigits.append(dtmf);
    if(dtmfDigits.isEmpty())
        return;

    digits->setText(dtmfDigits);
    digits->setCursorPosition(digits->text().length());
    digits->show();

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
        item->setValue( "State", state );
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
        if( item->call().dropped() )
            item->setValue("State", CallScreen::tr("Disconnected", "call state"));
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
	m_callAudioHandler->callStateChanged(activeCount || holdCount 
										|| dialing || incoming );

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
            QSoftMenuBar::setLabel(listView, Qt::Key_Back, "phone/reject", tr("Send Busy"));
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

void CallScreen::requestFailed(const QPhoneCall &,QPhoneCall::Request r)
{
    QString title, text;
    if(r == QPhoneCall::HoldFailed) {
        title = tr("Hold");
        text = tr("Hold attempt failed");
    } else {
        title = tr("Join/Transfer");
        text = tr("Join/transfer attempt failed");
    }

    QAbstractMessageBox *box = QAbstractMessageBox::messageBox(0, title, text, QAbstractMessageBox::Warning);
    box->setTimeout(3000, QAbstractMessageBox::NoButton);
    QtopiaApplication::execDialog(box);
    delete box;
}

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
        if (secondaryCallScreen)
            secondaryCallScreen->hide();
    } else {
        m->triggerUpdate();
    }
}

void CallScreen::splitCall()
{
    setWindowTitle(tr("Split Call","split 2 phone lines after having joined them"));
    setSelectMode(true);
    actionHold->setVisible(false);
    actionResume->setVisible(false);
    actionEnd->setVisible(false);
    actionEndAll->setVisible(false);
    actionMerge->setVisible(false);
    actionSplit->setVisible(false);
    actionTransfer->setVisible(false);
}

void CallScreen::callSelected(const QModelIndex& index)
{
    CallItemModel* m = qobject_cast<CallItemModel *>(listView->model());

    CallItemEntry *callItem = m->callItemEntry(index);
    if (!callItem )
        qWarning("CallScreen::callSelected(): invalid index passed to CallItemModel");
    if (m->flags(index) & Qt::ItemIsSelectable) {
        setSelectMode(false);
        //XXX I could be used for more than just split
        callItem->call().activate(QPhoneCall::CallOnly);
        setWindowTitle(tr("Calls"));
    }
}

void CallScreen::callClicked(const QModelIndex& index)
{
    CallItemModel* m = qobject_cast<CallItemModel *>(listView->model());
    CallItemEntry *callItem = m->callItemEntry(index);
    if (!callItem)
        return;

    if (listView->selectionMode() == QAbstractItemView::NoSelection) {
        /* Only perform these operations if touchscreen is the preferred method of input.
            This stops the user's ear putting a call on hold or accepting an incoming call etc.  */
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
}

void CallScreen::themeItemClicked(ThemeItem *item)
{
    if(!item)
        return;

    if( item->itemName().left( 11 ) == "keypad-show" ) {
        keypadVisible = true;
        manualLayout();
    }
    else if( item->itemName().left( 11 ) == "keypad-hide" )
    {
        keypadVisible = false;
        manualLayout();
        clearDtmfDigits();
    }
    else if( item->itemName() == "zero" )
    {
        dialNumbers("0");
    }
    else if( item->itemName() == "one" )
    {
        dialNumbers("1");
    }
    else if( item->itemName() == "two" )
    {
        dialNumbers("2");
    }
    else if( item->itemName() == "three" )
    {
        dialNumbers("3");
    }
    else if( item->itemName() == "four" )
    {
        dialNumbers("4");
    }
    else if( item->itemName() == "five" )
    {
        dialNumbers("5");
    }
    else if( item->itemName() == "six" )
    {
        dialNumbers("6");
    }
    else if( item->itemName() == "seven" )
    {
        dialNumbers("7");
    }
    else if( item->itemName() == "eight" )
    {
        dialNumbers("8");
    }
    else if( item->itemName() == "nine" )
    {
        dialNumbers("9");
    }
    else if( item->itemName() == "star" )
    {
        dialNumbers("*");
    }
    else if( item->itemName() == "hash" )
    {
        dialNumbers("#");
    }
}

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
    } else if (k->key() == Qt::Key_Call || k->key() == Qt::Key_Yes || k->key() == Qt::Key_F28) {
        if ( control->hasIncomingCall()) {
            emit acceptIncoming();
        } else {
            // send gsm keys
            if (!dtmfDigits.isEmpty()) {
                bool filtered = false;
                emit filterSelect(dtmfDigits, filtered);
                if (filtered)
                    clearDtmfDigits();
            }
        }
    } else if ((k->key() == Qt::Key_F28) && control->isConnected()) {
        control->endCall();
    } else {
        k->ignore();
    }
}

void CallScreen::showEvent( QShowEvent *e )
{
    if ( !updateTimer ) {
        updateTimer = new QTimer(this);
        connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateAll()));
    }
    updateTimer->start(1000);
    ThemedView::showEvent( e );
    manualLayout();
}

void CallScreen::keyReleaseEvent(QKeyEvent *k)
{
    if (k->key() == Qt::Key_Flip && control->hasIncomingCall()) {
        QSettings cfg("Trolltech","Phone");
        cfg.beginGroup("FlipFunction");
        if ( cfg.value("answer").toBool() )
            emit acceptIncoming();
    }
}

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

void CallScreen::hideEvent( QHideEvent * )
{
    if ( updateTimer )
        updateTimer->stop();
}

bool CallScreen::eventFilter(QObject *o, QEvent *e)
{
    if (o == listView) {
        if (e->type() == QEvent::KeyPress) {
            QKeyEvent *ke = (QKeyEvent *)e;
            if (listView->selectionMode() == QAbstractItemView::NoSelection) {
                if (ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down) {
                    return true;
                } else if (ke->key() == Qt::Key_Left) {
                    emit decreaseCallVolume();
                    return true;
                } else if (ke->key() == Qt::Key_Right) {
                    emit increaseCallVolume();
                    return true;
                } else if (ke->key() == Qt::Key_Select) {
                    // gsm key select
                    if (!dtmfDigits.isEmpty() && gsmActionTimer && gsmActionTimer->isActive()) {
                        bool filtered = false;
                        emit filterSelect(dtmfDigits, filtered);
                        if (filtered)
                            clearDtmfDigits();
                        return true;
                    }
                    if ( incoming ) {
                        if(control->incomingCall().startTime().secsTo(QDateTime::currentDateTime()) >= 1)
                            emit acceptIncoming();
                    } else {
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

                            if( !dialNumbers( text ) )
                                if ( !ke->isAutoRepeat() ) {
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

#include "callscreen.moc"
