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

#include "numberdisplay.h"
#include "quickdial.h"
#include "dialercontrol.h"
#include "qtopiaserverapplication.h"

#include <qtopia/pim/qphonenumber.h>
#include <qsoftmenubar.h>
#include <qtopiaapplication.h>
#include <quniqueid.h>
#include <qtopia/pim/qcontactmodel.h>

#include <QDialog>
#include <QLayout>
#include <QDebug>
#include <QKeyEvent>
#include <QSet>

// declare QuickDialModel
class QuickDialModel : public CallContactModel
{
    Q_OBJECT
public:
    QuickDialModel( QCallList& callList, QObject *parent = 0 );

public slots:
    void refresh();
    void populate();

private:
    QContactModel *clm;
};

// declare QuickDialContactView
class QuickDialContactView : public CallContactView
{
    Q_OBJECT
public:
    QuickDialContactView(QWidget *parent);

private slots:
    void selectedNumber(const QModelIndex& idx);

signals:
    void numberSelected(const QString&, const QUniqueId&);
};

QuickDialContactView::QuickDialContactView(QWidget *parent)
    : CallContactView(parent)
{
    connect( this, SIGNAL(activated(const QModelIndex&)), this, SLOT(selectedNumber(const QModelIndex&)) );
    connect( this, SIGNAL(clicked(const QModelIndex&)), this, SLOT(selectedNumber(const QModelIndex&)) );
}

void QuickDialContactView::selectedNumber(const QModelIndex& idx)
{
    /* sub view instead if QContact */
    CallContactItem* cci = cclm->itemAt(idx);
    if (cci && cci->type() == CallContactItem::Contact) {
        /* should be able to select from list of numbers */
        /* open dialog listing phone numbers of selected contact */
        QPhoneTypeSelector s(cci->contact(), QString());
        if (QtopiaApplication::execDialog(&s))
            emit numberSelected(s.selectedNumber(), QUniqueId());
    } else {
        emit numberSelected(cci->number(), QUniqueId());
    }
}

//---------------------------------------------------------------------------

QuickDialModel::QuickDialModel( QCallList& callList, QObject *parent )
    : CallContactModel( callList, parent ), clm(0)
{
}

void QuickDialModel::refresh()
{
    if (filter().isEmpty()) {
        CallContactModel::resetModel(); //delete existing items
        CallContactModel::refresh(); //reread CallListItems
        return;
    }

    if (!clm) {
        clm = new QContactModel(this);
        connect(clm, SIGNAL(modelReset()), this, SLOT(populate()));
    }

    QSettings config( "Trolltech", "Contacts" );

    // load SIM/No SIM settings.
    config.beginGroup( "default" );
    if (config.contains("SelectedSources/size")) {
        int count = config.beginReadArray("SelectedSources");
        QSet<QPimSource> set;
        for(int i = 0; i < count; ++i) {
            config.setArrayIndex(i);
            QPimSource s;
            s.context = QUuid(config.value("context").toString());
            s.identity = config.value("identity").toString();
            set.insert(s);
        }
        config.endArray();
        clm->setVisibleSources(set);
    }

    if (filter() == clm->filterText())
        populate();
    else
        clm->setFilter(filter(), QContactModel::ContainsPhoneNumber);
}

void QuickDialModel::populate()
{
    CallContactModel::resetModel(); //delete existing items
    CallContactModel::refresh(); //reread CallListItems

    if( clm->filterText().isEmpty() )
        return; //don't display anything if the filter is empty

    QList<QCallListItem> cl = mRawCallList;

    //create contacts that match - alphabetical order
    //const int numContacts = mAllContacts.count();
    const QString filStr = filter();
    const int filLen = filStr.length();


    int numMatchingEntries = 0; //limit number of entries in list
    int index = 0;
    QUniqueId cntid = clm->id(index++);
    while (!cntid.isNull())
    {
        QCallListItem clItem;
        CallContactItem* newItem = 0;

        // assumed matched by label...
        newItem = new CallContactItem(CallContactItem::Contact, clItem, this);
        newItem->setContact(clm, cntid);
        callContactItems.append(newItem);

        numMatchingEntries++;

        /* should only sow enough to fit on the screen,
           although perhaps have a way of searching out the rest
           later.  Scrolling through 3000+ item != quick */
        if (numMatchingEntries > 20)
            break;
        cntid = clm->id(index++);
    }

    //create remaining calllist items
    if (filLen==0 || filLen >= 3) {
        foreach(QCallListItem clItem, cl)
        {
            if( clItem.isNull() )
                continue;

            QCallListItem::CallType st = clItem.type();

            QString number = clItem.number();
            if( filStr.isEmpty() || (filLen >= 3 &&
                            (QPhoneNumber::matchPrefix( clItem.number(), filStr ) != 0)) )
            {
                CallContactItem::Types qdType;
                if( st == QCallListItem::Dialed )
                   qdType = CallContactItem::DialedNumber;
                else if( st == QCallListItem::Received )
                   qdType = CallContactItem::ReceivedCall;
                else if( st == QCallListItem::Missed )
                   qdType = CallContactItem::MissedCall;
                else
                   continue;

                CallContactItem *newItem = new CallContactItem(qdType, clItem, this);
                callContactItems.append(newItem);
            }
        }
    }
    reset();
}

//---------------------------------------------------------------------------

/*!
  \class PhoneQuickDialerScreen
  \brief The PhoneQuickDialerScreen class implements a keypad based dialer UI.
  \ingroup QtopiaServer::PhoneUI

  This class is a Qtopia \l{QtopiaServerApplication#qtopia-server-widgets}{server widget}. 

  \sa QAbstractServerInterface, QAbstractDialerScreen
  */


/*!
  \fn void PhoneQuickDialerScreen::numberSelected(const QString&, const QUniqueId&)

  \internal
*/

/*!
  Constructs a new PhoneQuickDialerScreen object with the specified
  \a parent and widget flags \a fl.
*/
PhoneQuickDialerScreen::PhoneQuickDialerScreen( QWidget *parent, Qt::WFlags fl )
    : QAbstractDialerScreen( parent, fl ), mSpeedDial( false )
{
    QCallList &callList = DialerControl::instance()->callList();
    QVBoxLayout *l = new QVBoxLayout( this );

    mNumberDS = new NumberDisplay( this );
    l->addWidget(mNumberDS);
    QtopiaApplication::setInputMethodHint( mNumberDS, QtopiaApplication::AlwaysOff );

    QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::Cancel );
    mDialList = new QuickDialContactView( this );
    l->addWidget(mDialList);
    mDialList->setEmptyMessage( tr("Type in phone number.") );

    mDialModel = new QuickDialModel(callList, mDialList);
    mDialList->setModel(mDialModel);
    CallContactDelegate * delegate = new CallContactDelegate( mDialList );
    mDialList->setItemDelegate( delegate );

    QItemSelectionModel * sm = mDialList->selectionModel();
    connect( sm, SIGNAL(currentChanged(const QModelIndex&,const QModelIndex&)),
        mDialList, SLOT(updateMenu(const QModelIndex&, const QModelIndex&)) );

    connect( mNumberDS, SIGNAL(numberChanged(const QString&)), this,
                                                    SLOT(rejectEmpty(const QString&)) );
    connect( mNumberDS, SIGNAL(numberChanged(const QString&)), mDialModel,
                                                    SLOT(setFilter(const QString&)) );
    connect( mNumberDS, SIGNAL(speedDialed(const QString&)), this,
                                    SIGNAL(speedDial(const QString&)) );
    connect( mNumberDS, SIGNAL(numberSelected(const QString&)), this,
                                    SLOT(selectedNumber(const QString&)) );
    connect( mNumberDS, SIGNAL(hangupActivated()), this, SLOT(close()) );

    connect( this, SIGNAL(numberSelected(const QString&, const QUniqueId&)), this,
                                            SIGNAL(requestDial(const QString&, const QUniqueId&)) );

    connect( mDialList, SIGNAL(requestedDial(const QString&, const QUniqueId&)), mDialList,
                                    SIGNAL(numberSelected(const QString&, const QUniqueId&)) );
    connect( mDialList, SIGNAL(numberSelected(const QString&, const QUniqueId&)), this,
                                    SLOT(selectedNumber(const QString&, const QUniqueId)) );
    connect( mDialList, SIGNAL(hangupActivated()), this, SLOT(close()) );
    setWindowTitle( tr("Quick Dial") );

    mNumberDS->installEventFilter( this );
    mDialList->installEventFilter( this );
    // Set the dialog to the maximum possible size.
}

/*!
  Destroys the PhoneQuickDialerScreen object.
  */
PhoneQuickDialerScreen::~PhoneQuickDialerScreen()
{
}

/*! \internal */
void PhoneQuickDialerScreen::showEvent( QShowEvent *e )
{
    QAbstractDialerScreen::showEvent( e );
    if( mNumber.length() )
    {
        // append digits after show event
        mNumberDS->appendNumber( mNumber, mSpeedDial );
        mNumber = QString();
        mSpeedDial = false;
    }
}

/*! \internal */
void PhoneQuickDialerScreen::selectedNumber( const QString &num )
{
    selectedNumber( num, QUniqueId() );
}

/*! \internal */
void PhoneQuickDialerScreen::selectedNumber( const QString &num, const QUniqueId &cnt )
{
    if( num.isEmpty() )
    {
        close();
        return;
    }
    // Filter for special GSM key sequences.
    bool filtered = false;
    emit filterSelect( num, filtered );
    if ( filtered ) {
        mNumber = QString();
        close();
        return;
    }
    mNumber = num;
    close();
    emit numberSelected( mNumber, cnt );
}

/*! \internal */
bool PhoneQuickDialerScreen::eventFilter( QObject *o, QEvent *e )
{
    QEvent::Type t = e->type();
    if( t != QEvent::KeyPress )
        return false;
    QKeyEvent *ke = (QKeyEvent *)e;
    int key = ke->key();
    QChar ch( key );
    if( o == mDialList )
    {
        switch( key )
        {
            case Qt::Key_Up:
            {
                QItemSelectionModel * selectModel = mDialList->selectionModel();
                if( !mDialModel->rowCount() ||
                    selectModel->isSelected(mDialModel->index(0)) )
                {
                    selectModel->clear();
                    mNumberDS->setFocus();
                    mNumberDS->setEditFocus(true);
                    return true;
                }
                break;
            }
            case Qt::Key_Down:
            {
                QItemSelectionModel * selectModel = mDialList->selectionModel();
                if( !mDialModel->rowCount() ||
                        selectModel->isSelected(mDialModel->index(mDialModel->rowCount()-1)) )
                {
                    selectModel->clear();
                    mNumberDS->setFocus();
                    mNumberDS->setEditFocus(true);
                    return true;
                }
                break;
            }
            case Qt::Key_0:
            case Qt::Key_1:
            case Qt::Key_2:
            case Qt::Key_3:
            case Qt::Key_4:
            case Qt::Key_5:
            case Qt::Key_6:
            case Qt::Key_7:
            case Qt::Key_8:
            case Qt::Key_9:
            {
                mNumberDS->appendNumber( ke->text() );
                mNumberDS->setFocus();
                mNumberDS->setEditFocus(true);
                return true;
            }
            case Qt::Key_Backspace:
            {
                mNumberDS->setFocus();
                mNumberDS->setEditFocus(true);
                QString curNumber = mNumberDS->number();
                if( !curNumber.isEmpty() )
                    mNumberDS->backspace();
                return true;
            }
            default:
                break;
        }
    }
    else if( o == mNumberDS )
    {
        QModelIndex idx;
        if( mDialModel->rowCount() && key == Qt::Key_Down )
            idx = mDialModel->index(0);
        else if( mDialModel->rowCount() && key == Qt::Key_Up )
            idx = mDialModel->index(mDialModel->rowCount()-1);
        if (idx.isValid()) {
            mDialList->setFocus();
            mDialList->setCurrentIndex(idx);
            mDialList->setEditFocus(true);
            return true;
        }
    }
    return false;
}

/*! \reimp */
void PhoneQuickDialerScreen::reset()
{
    mNumberDS->setNumber("");
    mNumberDS->setFocus();
    mNumber = QString();
}

/*! \internal */
void PhoneQuickDialerScreen::rejectEmpty( const QString &t )
{
    if( t.isEmpty() && !isVisible() )
        close();
    else {
        // Fitler special GSM key sequences that act immediately (e.g. *#06#).
        bool filtered = false;
        emit filterKeys( t, filtered );
        if ( filtered ) {
            mNumber = QString();
            close();
        }
    }
}

/*! \internal */
void PhoneQuickDialerScreen::appendDigits( const QString &digits, bool refresh,
                              bool speedDial )
{
    if( !refresh && isVisible() )
        qWarning("BUG: appending digits that will never be seen to quick dial");
    if( refresh )
        mNumberDS->appendNumber( digits, speedDial );
    else {
        mSpeedDial = mSpeedDial | speedDial;
        mNumber += digits;
    }
}

/*! \reimp */
QString PhoneQuickDialerScreen::digits() const
{
    if( mNumber.isEmpty() )
        return mNumberDS->number();
    return mNumber;
}

/*! \reimp */
void PhoneQuickDialerScreen::setDigits(const QString &digits)
{
    reset();
    appendDigits(digits, false, false);
}

/*! \reimp */
void PhoneQuickDialerScreen::appendDigits(const QString &digits)
{
    appendDigits(digits, false, true);
}

QTOPIA_REPLACE_WIDGET_WHEN(QAbstractDialerScreen, PhoneQuickDialerScreen, Keypad);

#include "quickdial.moc"

