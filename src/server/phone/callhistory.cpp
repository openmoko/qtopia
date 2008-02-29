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

#include <QTabWidget>
#include <QLayout>
#include <QAction>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QMessageBox>
#include <QTimer>
#include <QListWidget>
#include <QDebug>
#include <QLineEdit>
#include <QMenu>
#include <QTextFrame>
#include <QTextDocumentFragment>

#include <qtopianamespace.h>
#include <qtopia/pim/qphonenumber.h>

#include <qsoftmenubar.h>
#include <qtopiaapplication.h>
#include <qtopianamespace.h>
#include <qthumbnail.h>
#include <qtopiaservices.h>
#include <qtopiaipcenvelope.h>

#include "callhistory.h"
#include "savetocontacts.h"
#include "servercontactmodel.h"

// -------------------------------------------------------------
// CallHistoryModel

CallHistoryModel::CallHistoryModel( QCallList & callList, QObject* parent )
    : CallContactModel(callList, parent)
{
}

CallHistoryModel::~CallHistoryModel()
{
}

void CallHistoryModel::setType( CallContactItem::Types type )
{
    mType = type;
}

CallContactItem::Types CallHistoryModel::type() const
{
    return mType;
}
#include <QDebug>
void CallHistoryModel::refresh()
{
    CallContactModel::resetModel(); //delete existing items
    CallContactModel::refresh(); //reread CallListItems

    QList<QCallListItem> cl = mRawCallList;
    const QString filStr = filter();
    QContactModel *clm = ServerContactModel::instance();

    //go through all CallListItems
    foreach(QCallListItem  clItem, cl)
    {

        if (clItem.isNull())
            continue;

        QCallListItem::CallType st = clItem.type();

        if ( (int)CallContactItem::stateToType(st) == (int) type() ) {
            ;
        } else {
            continue; //skip items that aren't part of this list type
        }

        const QString number = clItem.number();
        const QUniqueId contactId = clItem.contact();

        //find all contacts that match the user-specified filter and this call item number
        bool hasContact = false;
        QContact cnt;

        if (!contactId.isNull()) {
            cnt = clm->contact( contactId );
        } else if (!number.isEmpty()) {
            cnt = clm->matchPhoneNumber( number );
        }

        if (!cnt.uid().isNull())  { // QContactModel::search does not match the exact number
            //can there be more than a single entry for one given number???
            if (filStr.isEmpty() || pk_matcher.collate(cnt.label()).startsWith(filStr)) {
                hasContact = true;
                CallContactItem * newItem = new CallContactItem(
                        CallContactItem::Contact, clItem, this);
                newItem->setContact(cnt, number);
                callContactItems.append(newItem);
            }
        }

        if ( !hasContact && (filStr.isEmpty() || QPhoneNumber::matchPrefix( number, filStr )))
        {
            //no contact associated to this QCallListItem
            CallContactItem * newItem = new CallContactItem(type(), clItem, this);
            callContactItems.append(newItem);
        }
    }
    reset(); //update views
}

// -------------------------------------------------------------
// CallHistoryList

CallHistoryList::CallHistoryList( QWidget *parent, Qt::WFlags /*fl*/ )
: CallContactView( parent ), mClearList( 0 )
{
}

void CallHistoryList::setModel(QAbstractItemModel *model)
{
    CallContactView::setModel(model);
    if(cclm)
    {
        connect( cclm, SIGNAL(modelReset()), this, SLOT(modelChanged()));
        connect( cclm, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(modelChanged()));
        connect( cclm, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(modelChanged()));
    }
}

void CallHistoryList::itemDeleted()
{
    if (cclm)
        cclm->refresh();
}

void CallHistoryList::updateMenu(const QModelIndex& current, const QModelIndex& previous)
{
    CallContactView::updateMenu( current, previous );
    if( mClearList )
        mMenu->addAction(mClearList);
}

void CallHistoryList::modelChanged()
{
    /* Hide the select button if we're empty - cclm should be valid to raise this signal*/
    if (cclm && cclm->rowCount() > 0) {
        QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::Select);
    } else {
        QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::NoLabel);
    }
}

// -------------------------------------------------------------
// CallHistoryBrowser

CallHistoryBrowser::CallHistoryBrowser( QWidget *parent )
    : QTextBrowser( parent )
{
}

void CallHistoryBrowser::focusFirstLink()
{
    focusNextChild();
}

void CallHistoryBrowser::keyPressEvent( QKeyEvent *e )
{
    QKeyEvent *oldEvent = 0;
    if( e->key() == Qt::Key_Yes || e->key() == Qt::Key_Call )
    {
        oldEvent = e;
        e = new QKeyEvent( QEvent::KeyPress, Qt::Key_Select, 0 );
    }
    QTextBrowser::keyPressEvent(e);
    if( oldEvent )
    {
        if( e->isAccepted() )
            oldEvent->accept();
        delete e;
        e = oldEvent;
    }
}

// -------------------------------------------------------------
// CallHistoryView

CallHistoryView::CallHistoryView( QCallList &callList, QWidget *parent, Qt::WFlags fl, bool rw )
    : QWidget( parent, fl ), mHaveFocus( false ), mHaveContact( false ),
      mPhoneType( QContactModel::Invalid ), mCallList( callList ), deleteMsg(0), addContactMsg(0), rView(0)
{
    mMenu = QSoftMenuBar::menuFor( this );

    QIcon addressbookIcon( ":icon/addressbook/AddressBook" );
    mOpenContact = new QAction( addressbookIcon, tr( "Open Contact" ), this );
    connect( mOpenContact, SIGNAL(triggered()), this, SLOT(openContact()) );
    mAddContact = new QAction( addressbookIcon, tr( "Save to Contacts" ), this );
    connect( mAddContact, SIGNAL(triggered()), this, SLOT(addContact()) );
    mSendMessage = new QAction( QIcon( ":icon/email" ), tr( "Send Message" ), this );
    connect( mSendMessage, SIGNAL(triggered()), this, SLOT(sendMessage()) );
    if ( rw ) {
        mDeleteAction = new QAction( QIcon( ":icon/trash" ), tr( "Delete" ), this );
        connect( mDeleteAction, SIGNAL(triggered()), this, SLOT(deleteItem()) );
    }
    else {
        mDeleteAction = 0;
    }
    updateMenu();

    //could assume this, but just to be sure the service is available
    mHaveDialer = !QtopiaService::channel("Dialer").isEmpty();
    QVBoxLayout *l = new QVBoxLayout( this );
    mBrowser = new CallHistoryBrowser( this );
    mBrowser->setFrameStyle(QFrame::NoFrame);
    connect( mBrowser, SIGNAL(anchorClicked(const QUrl&)), this,
                                                SLOT(linkActivated(const QUrl&)) );
    l->addWidget( mBrowser );

    if (rw)
        connect( this, SIGNAL(relatedCallClicked(int)), this, SLOT(viewRelatedDetails(int)) );
    connect( this, SIGNAL(externalLinkActivated()), this, SLOT(close()) );

    setWindowTitle( tr("Call History") );
}

void CallHistoryView::openContact()
{
    if( mHaveContact ) {
        QtopiaServiceRequest req( "Contacts", "showContact(QUniqueId)" );
        req << mContact.uid();
        req.send();
    }
}

void CallHistoryView::addContact()
{
    if (!mHaveContact)
        SavePhoneNumberDialog::savePhoneNumber(mCallListItem.number());
}

void CallHistoryView::sendMessage()
{
    if( mHaveContact && (mPhoneType ==
                QContactModel::HomeMobile || mPhoneType == QContactModel::BusinessMobile) )
    {
        QString name = mContact.label();
        QString number;
        if (mPhoneType == QContactModel::HomeMobile)
            number = mContact.homeMobile();
        else
            number = mContact.businessMobile();

        if (!number.isEmpty()) {
            QtopiaServiceRequest req( "SMS", "writeSms(QString,QString)");
            req << name << number;
            req.send();
        }
    }
}

void CallHistoryView::updateMenu()
{
    mMenu->clear();
    if( mHaveContact )
    {
        if( mPhoneType == QContactModel::HomeMobile ||
            mPhoneType == QContactModel::BusinessMobile )
            mMenu->addAction(mSendMessage);
        mMenu->addAction(mOpenContact);
    }
    else
    {
        mMenu->addAction(mAddContact);
    }
    if( mDeleteAction )
        mMenu->addAction(mDeleteAction);
}

CallHistoryView::~CallHistoryView()
{
    if( rView )
        delete rView;
}

void CallHistoryView::deleteItem()
{
    if( !mDeleteAction )
        return;

    delete deleteMsg;
    deleteMsg = QAbstractMessageBox::messageBox( this, tr("Delete Call History Item"),
        "<qt>" + tr("Are you sure you want to delete this item from the call history?") + "</qt>",
        QAbstractMessageBox::Warning, QAbstractMessageBox::Yes, QAbstractMessageBox::No );
    connect(deleteMsg, SIGNAL(finished(int)), this, SLOT(deleteItem(int)));
    QtopiaApplication::showDialog(deleteMsg);
}

void CallHistoryView::deleteItem(int r)
{
    if ( r == QAbstractMessageBox::Yes ) {
        for( int i = mCallList.count()-1; i >= 0; i--)
        {
            if( mCallListItem == mCallList.at(i) )
            {
                mCallList.removeAt( i );
                break;
            }
        }
        close();
        emit itemDeleted();
    }
}

QContact CallHistoryView::contact( QContactModel::Field &phoneType ) const
{
    phoneType = mPhoneType;
    return mContact;
}

void CallHistoryView::setContact( const QContact &cnt, QContactModel::Field phoneType )
{
    mContact = cnt;
    mPhoneType = phoneType;
    mHaveContact = true;
    updateMenu();
}

void CallHistoryView::setCallListItem( QCallListItem item )
{
    mCallListItem = item;
    rStack.push(item);
    updateMenu();
}

void CallHistoryView::clear()
{
    mHaveContact = false;
    mBrowser->setPlainText( QString() );
    mContact = QContact();
    mCallListItem = QCallListItem();
    updateMenu();
}

QString CallHistoryView::makeDialLink( const QString &number ) const
{
    if( !mHaveDialer )
        return number;
    QString link( "<a href=\"Dialer:%1\">%2</a>" );
    return link.arg( number ).arg( number );
}

void CallHistoryView::linkActivated( const QUrl &url )
{
    QString ahref = url.toString();
    if( ahref.startsWith( "Dialer:" ) )
    {
        QString number = ahref.mid( 7 );
        QtopiaServiceRequest request( "Dialer", "dial(QString,QUniqueId)" );
        request << number << mContact.uid();
        request.send();
        emit externalLinkActivated();
    }
    else if( ahref.startsWith( "callhistory:" ) )
    {
        int id = ahref.mid( 12 ).toInt();
        emit relatedCallClicked( id );
    }
}

// -------------------------------------------------------------
// RelatedCallListItem

//used to map a related call list item to its position in the call list
class RelatedCallListItem
{
public:
    RelatedCallListItem();
    RelatedCallListItem( QCallListItem clItem, int clIdx, QContactModel::Field pt = QContactModel::Invalid );
    RelatedCallListItem( const RelatedCallListItem &other );
    RelatedCallListItem &operator=( const RelatedCallListItem &other );
    QCallListItem item;
    int callListIndex;
    QContactModel::Field phoneType;
};

RelatedCallListItem::RelatedCallListItem()
    : callListIndex( -1 ), phoneType( QContactModel::Invalid )
{
}

RelatedCallListItem::RelatedCallListItem( QCallListItem clItem, int clIdx, QContactModel::Field pt )
    : item( clItem ), callListIndex( clIdx ), phoneType( pt )
{
}


RelatedCallListItem::RelatedCallListItem( const RelatedCallListItem &other )
{
    item = other.item;
    callListIndex = other.callListIndex;
    phoneType = other.phoneType;
}

RelatedCallListItem &RelatedCallListItem::operator=( const RelatedCallListItem &other )
{
    item = other.item;
    callListIndex = other.callListIndex;
    phoneType = other.phoneType;
    return *this;
}

void CallHistoryView::viewRelatedDetails( int callListIndex )
{
    if( callListIndex < 0 || callListIndex > (int)mCallList.count() )
    {
        qWarning("BUG: Invalid callListIndex passed to CallHistory::viewRelatedDetails");
        return;
    }

    QCallListItem item = mCallList.at( callListIndex );
    if (!rView) {
        rView = new CallHistoryView( mCallList, 0, 0, false );
        connect( rView, SIGNAL(externalLinkActivated()), this, SIGNAL(externalLinkActivated()) );
        //connect( rView, SIGNAL(externalLinkActivated()), this, SLOT(close()) );
        connect( rView, SIGNAL(relatedCallClicked(int)), this, SLOT(viewRelatedDetails(int)) );
        QContactModel::Field pt;
        QContact cnt;
        cnt = contact( pt );
        if( pt != QContactModel::Invalid )
            rView->setContact( cnt, pt );
    }
    rView->setCallListItem( item );
    rView->update();
    rView->showMaximized();
}

//copied from QContactModel
QFont differentFont(const QFont& start, int step)
{
    int osize = QFontMetrics(start).lineSpacing();
    QFont f = start;
    for (int t=1; t<6; t++) {
        int newSize = f.pointSize() + step;
        if ( newSize > 0 )
            f.setPointSize(newSize);
        else
            return start; // we cannot find a font -> return old one
        step += step < 0 ? -1 : +1;
        QFontMetrics fm(f);
        if ( fm.lineSpacing() != osize )
            break;
    }
    return f;
}


void CallHistoryView::update()
{
    QTextDocument* document = mBrowser->document();
    document->clear();
    document->setDefaultFont(font());
    QTextCursor cursor(document);

    //initialise character formats
    QTextCharFormat defaultFormat;
    QTextCharFormat headingFormat;
    headingFormat.setFontWeight(QFont::Bold);
    QTextCharFormat italicFormat;
    italicFormat.setFontItalic( true );
    QTextCharFormat linkFormat = defaultFormat;
    linkFormat.setAnchor( true );
    linkFormat.setFontUnderline( true );
    linkFormat.setUnderlineColor(palette().color(QPalette::Link));
    QTextCharFormat smallFormat = defaultFormat;
    QFont f = differentFont(smallFormat.font(), -3);
    smallFormat.setFont(f);

    QTextBlockFormat bf = cursor.blockFormat();
    bf.setAlignment(Qt::AlignHCenter);
    bf.setLeftMargin( 4 );
    cursor.setBlockFormat(bf);
    cursor.insertBlock(); // don't start at the top of the page

    if ( mHaveContact )
    {
        QContactModel contactModel;

        QString photoFileName = mContact.portraitFile();

        QString baseDirStr = Qtopia::applicationFileName( "addressbook", "contactimages/" );

        QTextImageFormat portrait;
        if ( !photoFileName.isEmpty() ) {
            portrait.setName(baseDirStr + photoFileName);
        } else {
            if ( contactModel.isSIMCardContact(mContact.uid()) )
                portrait.setName(":icon/addressbook/sim-contact");
            else
                portrait.setName(":icon/addressbook/generic-contact");
        }
        cursor.insertImage(portrait);

        cursor.insertText(mContact.label(), headingFormat);
        cursor.insertBlock();
        cursor.insertBlock();
    }

    //show number and phone type pic (if existing)
    if (mHaveDialer)
        linkFormat.setAnchorHref(QString("Dialer:%1").arg(mCallListItem.number()));
    cursor.insertText(mCallListItem.number(), linkFormat);

    if ( mHaveContact && mPhoneType != QContactModel::Invalid )
    {
        QString pixFileName = QContactModel::fieldIdentifier( mPhoneType );
        if (!pixFileName.isEmpty()) {
            pixFileName.prepend(":icon/addressbook/");
            QTextImageFormat pic;
            pic.setName(pixFileName);
            cursor.insertImage(pic);
        }
    }

    // show call information
    cursor.insertBlock();
    cursor.insertBlock();
    bf.setAlignment(Qt::AlignLeft);
    cursor.setBlockFormat(bf);
    QString value;
    if( mCallListItem.type() == QCallListItem::Received )
        value = tr("Received");
    else if( mCallListItem.type() == QCallListItem::Missed )
        value = tr("Missed");
    else
        value = tr("Dialed");
    cursor.insertText(value, headingFormat);
    cursor.insertBlock();
    cursor.insertText(QTimeString::localYMDHMS(mCallListItem.start(), QTimeString::Long), defaultFormat);
    cursor.insertBlock();

    if( !mCallListItem.end().isNull() && mCallListItem.type() != QCallListItem::Missed )
    {
        cursor.insertText(tr("Ended"), headingFormat);
        cursor.insertBlock();

        //only include date information again if it's different from the start date
        if ( mCallListItem.start().date() != mCallListItem.end().date() )
            cursor.insertText(QTimeString::localYMDHMS(mCallListItem.end(), QTimeString::Long), defaultFormat);
        else
            cursor.insertText( tr("Today, %1", "%1 = time")
                    .arg(QTimeString::localHMS(mCallListItem.end().time())), defaultFormat);
        cursor.insertBlock();

        cursor.insertText(tr("Duration"), headingFormat);
        cursor.insertBlock();

        const int SECS_PER_HOUR= 3600;
        const int SECS_PER_MIN  = 60;
        int duration = mCallListItem.start().secsTo( mCallListItem.end() );
        int hour = duration/SECS_PER_HOUR;
        int minute = (duration % SECS_PER_HOUR)/SECS_PER_MIN;
        int second = duration % SECS_PER_MIN;
        QString buf;
        buf.sprintf( "%.2d:%.2d:%.2d", hour, minute, second );
        cursor.insertText(buf, defaultFormat);
    }
    else if( mCallListItem.type() == QCallListItem::Dialed )
    {
        cursor.insertText(tr("Did not connect"), italicFormat);
    }

    cursor.insertBlock();

    //related calls information
    //related means other numbers in the call list belonging to this contact otherwise
    //other items of the same number
    QList<RelatedCallListItem> relatedCalls;
    int thisPos = -1;
    for( int i = 0 ; i < (int)mCallList.count() ; ++i )
    {
        if( mHaveContact )
        {
            QList<QContactModel::Field> phoneKeys = QContactModel::phoneFields();
            QString callNumber = mCallList.at(i).number() ;
            QString num;

            foreach (QContactModel::Field key, phoneKeys)
            {
                num = QContactModel::contactField(mContact, key).toString();
                if (!num.isEmpty()&& QPhoneNumber::matchNumbers( num, callNumber ))
                {
                    relatedCalls.append( RelatedCallListItem( mCallList.at(i), i,
                                key ));
                }
            }
        }
        else if( QPhoneNumber::matchNumbers( mCallList.at( i ).number(),
                                                            mCallListItem.number() ) )
        {
            //match call list number to other items
            relatedCalls.append( RelatedCallListItem( mCallList.at( i ), i ) );
        }
        if( mCallListItem == mCallList.at( i ) )
        {
            thisPos = relatedCalls.count()-1;
        }
    }
    if( relatedCalls.count() && thisPos == -1 )
        qWarning("BUG: thisPos == -1");
    if( relatedCalls.count() )
    {
        //get up to 10 calls within the context of mCallListItem
        const int maxContext = 6;
        int contextCount = 0;
        QList<RelatedCallListItem> relatedCallsInContext;
        QList<RelatedCallListItem> newerCallsInContext;
        QList<RelatedCallListItem> olderCallsInContext;
        //j is going up the list - older items
        //i is going down the list - newer items
        int i = thisPos-1, j = thisPos+1;
        while( contextCount != maxContext && (i >= 0 || j < relatedCalls.count()) )
        {
            //try and get maxContext items in either direction
            if( i >= 0 )
            {
                newerCallsInContext.prepend( relatedCalls[i--] );
                ++contextCount;
            }
            if( j < relatedCalls.count() )
            {
                olderCallsInContext.append( relatedCalls[j++] );
                ++contextCount;
            }
        }
        relatedCallsInContext = newerCallsInContext + olderCallsInContext;
        if( relatedCallsInContext.count() )
        {
            cursor.insertBlock();
            QTextBlockFormat bf = cursor.blockFormat();
            bf.setAlignment(Qt::AlignHCenter);
            cursor.setBlockFormat(bf);
            cursor.insertText(tr("Related Calls",
                       "Refers to other calls in the history from the same contact/number" ), headingFormat);
            cursor.insertBlock();
            cursor.insertBlock();
        }
        foreach(RelatedCallListItem curItem, relatedCallsInContext)
        {
            QString typePixmap;
            switch( curItem.item.type() )
            {
                case QCallListItem::Dialed:
                    typePixmap = "outgoingcall";
                    break;
                case QCallListItem::Received:
                    typePixmap = "incomingcall";
                    break;
                case QCallListItem::Missed:
                    typePixmap = "missedcall";
                    break;
            }
            typePixmap = Qt::escape(QString(":icon/phone/" + typePixmap ));
            QTextImageFormat callTypePic;
            callTypePic.setName(typePixmap);
            cursor.insertImage(callTypePic);

            QString callListIndex = QString::number( curItem.callListIndex );
            if( curItem.phoneType != QContactModel::Invalid && !mHaveContact )
                qWarning("BUa:G CallHistory::update : Have phoneType without contact");
            QString identifier, pixFile;
            if( curItem.phoneType != QContactModel::Invalid )
            {
                identifier = mContact.label();
                pixFile = QContactModel::fieldIdentifier( curItem.phoneType );
                pixFile.prepend(":icon/addressbook/");
            }
            else
                identifier = curItem.item.number();

            linkFormat.setAnchorHref(QString("callhistory:%1").arg(curItem.callListIndex));
            cursor.insertText(identifier, linkFormat);
            if (!pixFile.isEmpty())
            {
                QTextImageFormat pix;
                pix.setName(pixFile);
                cursor.insertImage(pix);
            }

            cursor.insertBlock();
            cursor.insertText(QTimeString::localYMDHMS(curItem.item.start(), QTimeString::Long), smallFormat);

            cursor.insertBlock();
            cursor.insertBlock();
        }
    }
    mBrowser->focusFirstLink();
}

void CallHistoryView::closeEvent(QCloseEvent *e)
{
    rStack.pop();
    if (!rStack.isEmpty()) {
        mCallListItem = rStack.top();
        update();
        e->ignore();
    } else {
        QWidget::closeEvent(e);
    }
}

class FriendlyTabs : public QTabWidget
{
    friend class CallHistory;
public:
    FriendlyTabs( QWidget *parent = 0 ) : QTabWidget( parent ) {}
};

// -------------------------------------------------------------
// CallHistoryClearList

CallHistoryClearList::CallHistoryClearList( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    setModal( true );
    QVBoxLayout *l = new QVBoxLayout( this );
    mList = new QListWidget( this );
    l->addWidget( mList );

    QListWidgetItem *item = 0;
    item = new QListWidgetItem(tr("Dialed History"));
    item->setIcon(QIcon( ":icon/phone/outgoingcall" ));
    mList->insertItem(CallHistoryClearList::Dialed, item);
    item = new QListWidgetItem(tr("Received History"));
    item->setIcon(QIcon( ":icon/phone/incomingcall" ));
    mList->insertItem(CallHistoryClearList::Received, item);
    item = new QListWidgetItem(tr("Missed History"));
    item->setIcon(QIcon( ":icon/phone/missedcall" ));
    mList->insertItem(CallHistoryClearList::Missed, item);
    item = new QListWidgetItem(tr("All History"));
    item->setIcon(QIcon( ":icon/callhistory/CallHistory" ));
    mList->insertItem(CallHistoryClearList::All, item);

    connect( mList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(userSelected(QListWidgetItem*)) );

    setWindowTitle( tr("Clear Call History") );
}

void CallHistoryClearList::userSelected(QListWidgetItem * item)
{
    close();
    emit selected((CallHistoryClearList::ClearType)(mList->row(item)));
}

void CallHistoryClearList::setSelected( CallHistoryClearList::ClearType type )
{
    mList->setCurrentRow( (int)type );
}

CallHistoryClearList::~CallHistoryClearList()
{
}

// -------------------------------------------------------------
// CallHistory

CallHistory::CallHistory( QCallList &callList, QWidget *parent, Qt::WFlags fl )
    : QWidget( parent, fl ), mView( 0 ), mCallList( callList ), mShowMissedCalls( false ),
      mDialedListShown( false ), mReceivedListShown( false ), mMissedListShown( false ), mClearList( 0 ), mDialedFindLE( 0 ), mReceivedFindLE( 0 ), mMissedFindLE( 0 )

{
    mDialedList = 0;
    mReceivedList = 0;
    mMissedList = 0;
    mDialedFindLE = 0;
    mReceivedFindLE = 0;
    mMissedFindLE = 0;
    mView = 0;

    mTabs = new FriendlyTabs( this );
    QWidget *b = (QWidget *)mTabs->tabBar();
    b->installEventFilter( this );
    QMenu *m = QSoftMenuBar::menuFor( b );
    QAction *clearList = new QAction(QIcon(":icon/trash"),
            tr("Clear..."), this );
    m->addAction(clearList);
    connect( clearList, SIGNAL(triggered()), this, SLOT(clearList()) );

    QVBoxLayout *l = new QVBoxLayout( this );
    l->addWidget( mTabs );
    connect( mTabs, SIGNAL(currentChanged(int)), this, SLOT(refreshOnFirstShow(int)) );
    QWidget *par = mTabs;

    QVBoxLayout *wl = 0;
    QWidget *tab = 0; // only used on touchscreen
    if( Qtopia::mousePreferred() )
    {
        //recently dialed
        tab = new QWidget( mTabs );
        wl = new QVBoxLayout( tab );
        mDialedFindLE = new QLineEdit( tab );
        QtopiaApplication::setInputMethodHint( mDialedFindLE, "addressbook-names" );
        mDialedFindLE->installEventFilter( this );
        connect( mDialedFindLE, SIGNAL(textChanged(const QString&)), this, SLOT(setFilterCur(const QString&)) );
        wl->addWidget( mDialedFindLE );
        mDialedTab = tab;
        par = tab;
    }
    else
    {
        par = mTabs;
    }


    QItemSelectionModel* selectModel = 0;
    CallHistoryModel * cclm = 0;
    CallContactDelegate* delegate = new CallContactDelegate(mTabs);

    mDialedList = new CallHistoryList( par );
    cclm = new CallHistoryModel(callList, mDialedList);
    mDialedList->setModel( cclm );
    mDialedList->setItemDelegate( delegate );
    selectModel = mDialedList->selectionModel();
    connect( selectModel, SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
                mDialedList, SLOT(updateMenu(const QModelIndex&, const QModelIndex&)));

    if( Qtopia::mousePreferred() )
        mDialedList->setFocusPolicy( Qt::NoFocus );
    else
        mDialedTab = mDialedList;

    cclm->setType( CallContactItem::DialedNumber );
    mDialedList->mClearList = clearList;
    if( Qtopia::mousePreferred() )
        wl->addWidget( mDialedList );

    mTabs->addTab( mDialedTab, QIcon( ":icon/phone/outgoingcall" ), "" );

    if( Qtopia::mousePreferred() )
    {
        //recently called
        tab = new QWidget( mTabs );
        wl = new QVBoxLayout( tab );
        mReceivedFindLE = new QLineEdit( tab );
        QtopiaApplication::setInputMethodHint( mReceivedFindLE, "addressbook-names");
        mReceivedFindLE->installEventFilter( this );
        connect( mReceivedFindLE, SIGNAL(textChanged(const QString&)), this, SLOT(setFilterCur(const QString&)) );
        wl->addWidget( mReceivedFindLE );
        mReceivedTab = tab;
        par = tab;
    }
    else
    {
        par = mTabs;
    }

    mReceivedList = new CallHistoryList( par );
    cclm = new CallHistoryModel(callList, mReceivedList);
    mReceivedList->setModel( cclm );
    mReceivedList->setItemDelegate( delegate );
    selectModel = mReceivedList->selectionModel();
    connect( selectModel, SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
                mReceivedList, SLOT(updateMenu(const QModelIndex&, const QModelIndex&)));
    if( Qtopia::mousePreferred() )
        mReceivedList->setFocusPolicy( Qt::NoFocus );
    else
        mReceivedTab = mReceivedList;
    cclm->setType( CallContactItem::ReceivedCall );
    mReceivedList->mClearList = clearList;
    if( Qtopia::mousePreferred() )
        wl->addWidget( mReceivedList );

    mTabs->addTab( mReceivedTab, QIcon( ":icon/phone/incomingcall" ), "" );

    if( Qtopia::mousePreferred() )
    {
        //missed calls
        tab = new QWidget( mTabs );
        wl = new QVBoxLayout( tab );
        mMissedFindLE = new QLineEdit( tab );
        QtopiaApplication::setInputMethodHint( mMissedFindLE, "addressbook-names" );
        mMissedFindLE->installEventFilter( this );
        connect( mMissedFindLE, SIGNAL(textChanged(const QString&)), this, SLOT(setFilterCur(const QString&)) );
        wl->addWidget( mMissedFindLE );
        mMissedTab = tab;
        par = tab;
    }
    else
    {
        par = mTabs;
    }

    mMissedList = new CallHistoryList( par );
    cclm = new CallHistoryModel(callList, mMissedList);
    mMissedList->setModel( cclm );
    mMissedList->setItemDelegate( delegate );
    selectModel = mMissedList->selectionModel();
    connect( selectModel, SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
                mMissedList, SLOT(updateMenu(const QModelIndex&, const QModelIndex&)));
    if( Qtopia::mousePreferred() )
        mMissedList->setFocusPolicy( Qt::NoFocus );
    else
        mMissedTab = mMissedList;
    cclm->setType( CallContactItem::MissedCall );
    mMissedList->mClearList = clearList;
    if( Qtopia::mousePreferred() )
        wl->addWidget( mMissedList );

    mTabs->addTab( mMissedTab, QIcon( ":icon/phone/missedcall" ), "" );

    connect( mDialedList, SIGNAL(requestedDial(const QString&, const QUniqueId &)), this,
                                            SIGNAL(requestedDial(const QString&, const QUniqueId &)) );
    connect( mReceivedList, SIGNAL(requestedDial(const QString&, const QUniqueId &)), this,
                                            SIGNAL(requestedDial(const QString&, const QUniqueId &)) );
    connect( mMissedList, SIGNAL(requestedDial(const QString&, const QUniqueId &)), this,
                                            SIGNAL(requestedDial(const QString&, const QUniqueId &)) );
    connect( mDialedList, SIGNAL(hangupActivated()), this,
                                            SLOT(close()) );
    connect( mReceivedList, SIGNAL(hangupActivated()), this,
                                            SLOT(close()) );
    connect( mMissedList, SIGNAL(hangupActivated()), this,
                                            SLOT(close()) );

    connect( mDialedList, SIGNAL(activated(const QModelIndex&)), this, SLOT(viewDetails(const QModelIndex&)) );
    connect( mDialedList, SIGNAL(clicked(const QModelIndex&)), this, SLOT(viewDetails(const QModelIndex&)) );
    connect( mReceivedList, SIGNAL(activated(const QModelIndex&)), this, SLOT(viewDetails(const QModelIndex&)) );
    connect( mReceivedList, SIGNAL(clicked(const QModelIndex&)), this, SLOT(viewDetails(const QModelIndex&)) );
    connect( mMissedList, SIGNAL(activated(const QModelIndex&)), this, SLOT(viewDetails(const QModelIndex&)) );
    connect( mMissedList, SIGNAL(clicked(const QModelIndex&)), this, SLOT(viewDetails(const QModelIndex&)) );
    connect( this, SIGNAL(requestedDial(const QString&, const QUniqueId &)), this, SLOT(close()) );
    connect( mTabs, SIGNAL(currentChanged(int)), this, SLOT(pageChanged(int)));

    mDialedList->installEventFilter( this );
    mReceivedList->installEventFilter( this );
    mMissedList->installEventFilter( this );

    setWindowTitle( tr("Call History") );
    setObjectName("callhistory");

    if( Qtopia::mousePreferred() )
        mTabs->setFocusPolicy( Qt::NoFocus );
}

void CallHistory::refreshOnFirstShow(int index)
{
    QWidget *w = mTabs->widget(index);
    CallHistoryList * list = 0;

    if (w == mDialedTab && !mDialedListShown) {
        mDialedListShown = true;
        list = mDialedList;
    } else if (w == mMissedTab && !mMissedListShown) {
        mMissedListShown = true;
        list = mMissedList;
    } else if (w == mReceivedTab && !mReceivedListShown) {
        mReceivedListShown = true;
        list = mReceivedList;
    } else
        return;

    CallHistoryModel* chm = qobject_cast<CallHistoryModel*>(list->model());
    if (chm) {
        chm->refresh();
        list->setFocus();
        if (chm->rowCount())
            list->setCurrentIndex(chm->index(0));
    }
}

void CallHistory::setFilterCur( const QString &f )
{
    if( Qtopia::mousePreferred() )
    {
        const QObject *s = sender();
        CallHistoryList *list = 0;
        if( s == mDialedFindLE )
            list = mDialedList;
        else if( s == mReceivedFindLE )
            list = mReceivedList;
        else if( s == mMissedFindLE )
            list = mMissedList;
        if( list ) {
            CallHistoryModel* chm = qobject_cast<CallHistoryModel*>(list->model());
            if (chm)
                chm->setFilter( f );
        }
    }
}

void CallHistory::clearList()
{
    if( !mClearList )
    {
        mClearList = new CallHistoryClearList( this );
        QtopiaApplication::setMenuLike( mClearList, true );
        connect( mClearList, SIGNAL(selected(CallHistoryClearList::ClearType)), this, SLOT(clearList(CallHistoryClearList::ClearType)) );
    }

    switch( mTabs->currentIndex() )
    {
        case 0:
        case 1:
        case 2:
            mClearList->setSelected( (CallHistoryClearList::ClearType)mTabs->currentIndex() );
            break;
        default:
            mClearList->setSelected( CallHistoryClearList::All );
            break;
    }

    QtopiaApplication::showDialog( mClearList );
}

void CallHistory::clearList( CallHistoryClearList::ClearType type )
{
    QList<CallHistoryList*> changedLists;

    QString itemToDelete = tr("all %1 items", "%1 is either dialed, received, or missed");
    QString itemType;
    switch (type)
    {
        case CallHistoryClearList::Dialed:
            itemType = tr("dialed", "inserted into the context: all ... items");
            break;
        case CallHistoryClearList::Received:
            itemType = tr("received", "inserted into the context: all ... items");
            break;
        case CallHistoryClearList::Missed:
            itemType = tr("missed", "inserted into the context: all ... items");
            break;
        default: // delete all item
            break;
    }
    if (itemType.isEmpty())
    {
        itemToDelete.remove("%1 ");
    }
    else
        itemToDelete = itemToDelete.arg(itemType);

    int confirmed = QAbstractMessageBox::warning( this, tr("Clear Call History"),
        "<qt>" + tr("Are you sure you want to delete: %1?", "%1=all dialed/received/missed call history items").arg(itemToDelete) + "</qt>",
         QAbstractMessageBox::Yes, QAbstractMessageBox::No );
    if ( confirmed != QAbstractMessageBox::Yes )
        return;

    if( type == CallHistoryClearList::All )
    {

        mCallList.clear();
        changedLists.append( mDialedList );
        changedLists.append( mReceivedList );
        changedLists.append( mMissedList );
    }
    else
    {

        QCallListItem::CallType ctype;
        if( type == CallHistoryClearList::Dialed )
            ctype = QCallListItem::Dialed;
        else if( type == CallHistoryClearList::Received )
            ctype = QCallListItem::Received;
        else
            ctype = QCallListItem::Missed;
        CallHistoryList *cl = 0;
        if( ctype == QCallListItem::Dialed )
            cl = mDialedList;
        else if( ctype == QCallListItem::Received )
            cl = mReceivedList;
        else if( ctype == QCallListItem::Missed )
            cl = mMissedList;
        if( cl )
            changedLists.append( cl );
        int i = 0;
        while( i < (int)mCallList.count() )
        {
            if( ctype == mCallList.at( i ).type() )
                mCallList.removeAt( i );
            else {
                switch(mCallList.at( i ).type())
                {
                    case QCallListItem::Missed:
                    case QCallListItem::Dialed:
                    case QCallListItem::Received:
                        break;
                    default:
                        qWarning() << "BUG: Unrecognised type of QCallListItem, deleting item" ;
                        mCallList.removeAt( i );
                }
                ++i;
            }
        }
    }

    foreach(CallHistoryList* list, changedLists)
    {
        CallHistoryModel* chm = qobject_cast<CallHistoryModel*>(list->model());
        if (chm)
            chm->refresh();
    }
}

void CallHistory::showEvent( QShowEvent *e )
{
    QWidget::showEvent( e );
    if (mShowMissedCalls) {
        mTabs->setCurrentIndex( 2 );
    } else {
        if (mTabs->currentIndex() == 0)
            refreshOnFirstShow(0);
        mTabs->setCurrentIndex( 0 );
    }
}

void CallHistory::closeEvent( QCloseEvent *e )
{
    QWidget::closeEvent(e);
    // cleanup views with a single shot, as closeEvent can occur
    // during a key event on the views and therefore deleting the views
    // here directly would result in a crash
    QTimer::singleShot( 0, this, SLOT(cleanup()) );
}

void CallHistory::cleanup()
{
    if (mView) {
        delete mView;
        mView = 0;
    }
}

void CallHistory::pageChanged(int index)
{
    if( mTabs->widget(index) == mMissedTab )
        emit viewedMissedCalls();
}

void CallHistory::viewDetails( const QModelIndex& idx )
{
    if (!idx.isValid())
        return;

    CallHistoryList *list = (CallHistoryList *)sender();
    if( !list )
        return;

    CallHistoryModel * model = qobject_cast<CallHistoryModel*>(list->model());
    if (!model)
        return;

    CallContactItem *item = model->itemAt(idx);
    if (!item)
        return;

    QCallListItem clItem = item->callListItem();
    if( !mView )
    {
        mView = new CallHistoryView( mCallList );
        mView->installEventFilter(this);
        connect( mView, SIGNAL(itemDeleted()), this, SLOT(update()) );
        connect( mView, SIGNAL(itemDeleted()), mDialedList, SLOT(itemDeleted()) );
        connect( mView, SIGNAL(itemDeleted()), mReceivedList, SLOT(itemDeleted()) );
        connect( mView, SIGNAL(itemDeleted()), mMissedList, SLOT(itemDeleted()) );
        connect( mView, SIGNAL(externalLinkActivated()), this, SLOT(close()) );
    }
    mView->clear();
    mView->setCallListItem( clItem );
    if( !(item->fieldType() == QContactModel::Invalid) )
        mView->setContact( item->contact(),
                                item->fieldType() );
    mView->showMaximized();
}

bool CallHistory::eventFilter( QObject *o, QEvent *e )
{
    QWidget *tb = (QWidget *)mTabs->tabBar();
    if( o == mDialedList ||
        o == mReceivedList ||
        o == mMissedList )
    {
        if( e->type() == QEvent::KeyPress )
        {

            QKeyEvent *ke = (QKeyEvent *)e;
            int key =  ke->key();
            QString text = ke->text();
            CallHistoryList* chList = qobject_cast<CallHistoryList*>(o);
            if (!chList)
                return false;
            CallHistoryModel* chModel = qobject_cast<CallHistoryModel*>(chList->model());
            if( !text.isEmpty() && chModel )
            {
                mFilters[o] += text;
                chModel->setFilter( mFilters[o] );
                QSoftMenuBar::setLabel( chList, Qt::Key_Back, QSoftMenuBar::BackSpace);
                return true;
            }

            int curPageIdx = mTabs->currentIndex();
            bool rtl = QtopiaApplication::layoutDirection() == Qt::RightToLeft;
            switch( key )
            {
                case Qt::Key_Left:
                {
                    if( !rtl && curPageIdx > 0 )
                        --curPageIdx;
                    else if ( rtl && curPageIdx < 2 )
                        ++curPageIdx;
                    mTabs->setCurrentIndex( curPageIdx );
                    return true;
                }
                case Qt::Key_Right:
                {
                    if( !rtl && curPageIdx < 2 )
                        ++curPageIdx;
                    else if ( rtl && curPageIdx > 0 )
                        --curPageIdx;
                    mTabs->setCurrentIndex( curPageIdx );
                    return true;
                }
                case Qt::Key_Up:
                {
                    if( chList->currentIndex().row() == 0 )
                    {
                        chList->selectionModel()->clear();
                        mTabs->setFocus();
                        return true;
                    }
                    break;
                }
                case Qt::Key_Down:
                {
                    if( chModel && chList->currentIndex().row() == chModel->rowCount()-1 )
                    {
                        chList->selectionModel()->clear();
                        mTabs->setFocus();
                        return true;
                    }
                    break;
                }
                case Qt::Key_No:
                case Qt::Key_Back:
                {
                    QString filter = mFilters[o];
                    if( !filter.isEmpty() && chModel )
                    {
                        //reset filter
                        mFilters[o] = "";
                        if( !Qtopia::mousePreferred() )
                            chModel->setFilter( "" );
                        else
                        {
                            if( chList == mDialedList )
                                mDialedFindLE->setText( QString() );
                            else if( chList == mReceivedList )
                                mReceivedFindLE->setText( QString() );
                            else if( chList == mMissedList )
                                mMissedFindLE->setText( QString() );
                        }
                        QSoftMenuBar::setLabel( chList, Qt::Key_Back,
                                QSoftMenuBar::Back, QSoftMenuBar::AnyFocus );
                        return true;
                    }
                    break;
                }
                case Qt::Key_Backspace:
                {
                    QString filter = mFilters[o];
                    if( !filter.isEmpty() && chModel )
                    {
                        filter = filter.left( filter.length()-1 );
                        chModel->setFilter( filter );
                        mFilters[o] = filter;
                        if( filter.isEmpty() )
                            QSoftMenuBar::setLabel( chList, Qt::Key_Back,
                                    QSoftMenuBar::Back, QSoftMenuBar::AnyFocus );
                    }
                    return true;
                }
                default:
                    break;
            }
        }
    }

    else if( Qtopia::mousePreferred() && (o == mDialedFindLE ||
            o == mReceivedFindLE ||
            o == mMissedFindLE) && e->type() == QEvent::KeyPress )
    {
        CallHistoryList *receiver = 0;
        if( o == mDialedFindLE )
            receiver = mDialedList;
        else if( o == mReceivedFindLE )
            receiver = mReceivedList;
        else
            receiver = mMissedList;
        QKeyEvent *ke = (QKeyEvent *)e;
        switch( ke->key() )
        {
            case Qt::Key_Select:
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Up:
            case Qt::Key_Down:
            {
                QtopiaApplication::postEvent( receiver, new QKeyEvent( ke->type(), ke->key(),
                        ke->modifiers(), ke->text(), ke->isAutoRepeat(), ke->count() ) );
                return true;
            }
            case Qt::Key_Call:
            case Qt::Key_Yes:
            {
                const QModelIndex idx = receiver->currentIndex();
                if (idx.isValid()) {
                    emit requestedDial( receiver->numberForIndex( idx ), receiver->contactForIndex( idx ).uid() );
                }
                return true;
            }
        }
    }
    else if( o == tb )
    {
        if( e->type() == QEvent::KeyPress )
        {
            QKeyEvent *ke = (QKeyEvent *)e;
            int key =  ke->key();
            CallHistoryList *list = 0;
            switch( mTabs->currentIndex() )
            {
                case 0:
                    list = mDialedList;
                    break;
                case 1:
                    list = mReceivedList;
                    break;
                case 2:
                    list = mMissedList;
                    break;
                default:
                    qWarning("BUG: CallHistory::eventFilter - Unknown page index %d", mTabs->currentIndex());
                    return false;
            }
            QAbstractListModel* model = qobject_cast<QAbstractListModel*>(list->model());
            if( model && model->rowCount() )
            {
                if( key == Qt::Key_Up )
                {
                    list->setFocus();
                    list->setCurrentIndex(model->index(model->rowCount()-1));
                    return true;
                }
                else if( key == Qt::Key_Down )
                {
                    list->setFocus();
                    list->setCurrentIndex(model->index(0));
                    return true;
                }
            }
        }
    } else if ( o == mView )
    {
        if (e->type() == QEvent::WindowActivate)
            mView->update();
    }
    return false;
}

void CallHistory::reset()
{
    mShowMissedCalls = false;
    mFilters.clear();
    mDialedListShown = mReceivedListShown = mMissedListShown = false;
    if( Qtopia::mousePreferred() )
    {
        mDialedFindLE->setText( QString() );
        mReceivedFindLE->setText( QString() );
        mMissedFindLE->setText( QString() );
    }
}

void CallHistory::setFilter( const QString &f )
{
    mFilters[mDialedList] = mFilters[mReceivedList] = mFilters[mMissedList] = f;
    if( !Qtopia::mousePreferred() )
    {
        CallHistoryModel *chm = qobject_cast<CallHistoryModel*>(mDialedList->model());
        if (chm)
           chm->setFilter( f );
        chm = qobject_cast<CallHistoryModel*>(mReceivedList->model());
        if (chm)
           chm->setFilter( f );
        chm = qobject_cast<CallHistoryModel*>(mMissedList->model());
        if (chm)
           chm->setFilter( f );
    }
    else
    {
        mDialedFindLE->setText( f );
        mReceivedFindLE->setText( f );
        mMissedFindLE->setText( f );
    }
}

void CallHistory::refresh()
{
    CallHistoryList *list = 0;
    switch( mTabs->currentIndex() )
    {
        case 0:
            list = mDialedList;
            mDialedListShown = true;
            break;
        case 1:
            list = mReceivedList;
            mReceivedListShown = true;
            break;
        case 2:
            list = mMissedList;
            mMissedListShown = true;
            break;
        default:
            qWarning("BUG: CallHistory::refresh - Unknown active page index %d", mTabs->currentIndex());
            return;
    }
    CallHistoryModel *chm = qobject_cast<CallHistoryModel*>(list->model());
    if (chm)
       chm->refresh();
}

void CallHistory::showMissedCalls()
{
    mShowMissedCalls = true;
}

