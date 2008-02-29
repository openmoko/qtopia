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



#include "email.h"
#include "maillistview.h"
#include "common.h"

#include <qapplication.h>
#include <qwhatsthis.h>

#include <qsettings.h>
#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#endif
#include <qtopianamespace.h>
#include <qdesktopwidget.h>
#include <qheaderview.h>
#include <qaction.h>
#include <qscrollbar.h>
#include <QKeyEvent>

// wrapper class to extract info from QTableWidget regarding sorting and
// position of each label in the header
MailListView::MailListView(QWidget *parent, const char *name)
    : QTableWidget( parent)
{
    setObjectName( name );
    setFrameStyle( NoFrame );
    connect(horizontalHeader(), SIGNAL(sectionResized(int,int,int)), this, SLOT(sizeChange(int,int,int)) );
    horizontalHeader()->setSortIndicatorShown(false);

    sortColumn = -1;
    ascending = false;

    connect( &menuTimer, SIGNAL( timeout() ), SLOT( itemMenuRequested() ) );
    connect( this, SIGNAL( itemSelectionChanged() ), SLOT( cancelMenuTimer() ) );
    connect( horizontalScrollBar(), SIGNAL( valueChanged(int) ),
             this, SLOT( scrollToLeft(int) ) );

    arrival = false;

    hVisible = true;    //part of sizeChangefix (avoid unnecessary flickering)
    maxColumnWidth = 0;
#ifdef QTOPIA_PHONE
    mSingleColumnMode = true;
    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
#else
    mSingleColumnMode = false;
#endif
    columns << "";
    columns << tr( "From" );
    columns <<  tr( "Subject" );
    columns <<  tr( "Date" );
    setColumnCount( columns.count() );
    setHorizontalHeaderLabels( columns );
    verticalHeader()->hide();
    int oldSize = verticalHeader()->defaultSectionSize();
    verticalHeader()->setDefaultSectionSize( oldSize * 5 / 4 );
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    setAlternatingRowColors( true );

    horizontalHeaderItem(0)->setIcon(QPixmap(":image/flag"));

    QAction *hhWhatsThis = QWhatsThis::createAction( horizontalHeader() );
    hhWhatsThis->setText( tr("Select the sort order by tapping any of the columns in this header.  Tapping the same column twice will switch between ascending and descending order.") );

    QAction *tWhatsThis = QWhatsThis::createAction( this );
    tWhatsThis->setText( tr("A list of the messages in your current folder.  Tap a mail to examine it.") );

//    setRootIsDecorated( true );

    setSelectionMode( QAbstractItemView::ExtendedSelection );
    EmailListItemDelegate *delegate = new EmailListItemDelegate( this );
    setItemDelegate( delegate );
    scrollToLeft( 0 );
    mContactModel = 0;
}

/*  Fix for a Qt-bug.  Item replacing the horizontal scrollbar is not
        painted when the headers are resized to fit within the viewport  */
void MailListView::sizeChange(int, int, int)
{
    //Let's just assume the bug is fixed in Qt4 for now -sanders
/*
    if ( !horizontalScrollBar()->isVisible() ) {
        if ( hVisible ) {
            triggerUpdate();
            hVisible = false;
        }
    } else {
        hVisible = true;
    }
*/
}

QSize MailListView::sizeHint() const
{
    return QSize(-1,-1);
}

QSize MailListView::minimumSizeHint() const
{
    return QSize(-1,-1);
}

void MailListView::keyPressEvent( QKeyEvent *e ) // sharp and phone
{
    switch( e->key() ) {
        case Qt::Key_Space:
        case Qt::Key_Return:
#ifdef QTOPIA_PHONE
        case Qt::Key_Select:
#endif
        case Qt::Key_Enter:
        {
            emit clicked(currentIndex());
        }
        break;
#ifdef QTOPIA_PHONE
        case Qt::Key_No:
        case Qt::Key_Back:
#endif
        case Qt::Key_Backspace:
        {
            //if (!Qtopia::mousePreferred())
                emit viewFolderList();
            //else
            //    e->ignore();
        }
        break;
        default:  QTableWidget::keyPressEvent( e );
    }
    if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) {
        clearSelection();
        selectRow( currentRow() );
    }
}

void MailListView::setSorting(int col, bool a)
{
    sortColumn = col;
    ascending = a;
    Qt::SortOrder order = a ? Qt::AscendingOrder : Qt::DescendingOrder;
    QTableWidget::sortItems(col, order);

    if ( currentItem() )
        scrollTo( currentIndex() );
}


/*  ArrivalDate sorts by the internalId, which is the order
        the mails arrived.  */
bool MailListView::arrivalDate()
{
    return arrival;
}

void MailListView::setByArrival(bool on)
{
    arrival = on;

    if ( arrival )
        horizontalHeaderItem(3)->setText( tr("Arrival") );
}

void MailListView::mousePressEvent( QMouseEvent * e )
{
    QTableWidget::mousePressEvent( e );
    menuTimer.setSingleShot( true );
    menuTimer.start( 500 );
}

void MailListView::mouseReleaseEvent( QMouseEvent * e )
{
    QTableWidget::mouseReleaseEvent( e );
    menuTimer.stop();
}

void MailListView::cancelMenuTimer()
{
    if( menuTimer.isActive() )
        menuTimer.stop();
}

void MailListView::itemMenuRequested()
{
    EmailListItem *item = (EmailListItem * ) currentItem();
    if ( item )
        emit itemPressed( item );
}

void MailListView::defineSort(int column, bool ascend)
{
    sortColumn = column;
    ascending = ascend;

    setSorting(sortColumn, ascending);
}

int MailListView::labelPos(int at)
{
    return ( horizontalHeader()->logicalIndex(at) );
}

void MailListView::moveSection(int section, int toIndex)
{
    horizontalHeader()->moveSection(section, toIndex);
}

int MailListView::sortedColumn()
{
    return sortColumn;
}

bool MailListView::isAscending()
{
    return ascending;
}

uint MailListView::getMailCount(QString type)
{
    int st = 0;
    if ( type == "new" ) // No tr
        st = 1;
    else if ( type == "unsent" ) // No tr
        st = 2;
    else if ( type == "unfinished" ) // No tr
        st = 3;

    if ( st == 0 )
        return rowCount();

    Email *mail;
    uint count = 0;
    for (int i = 0; i < rowCount(); ++i) {
        mail = ( (EmailListItem *) item(i, 0) )->mail();
        switch( st ) {
            case 1:
            {
                if ( !mail->status(EFlag_Read) )
                    count++;
                break;
            }
            case 2:
            {
                if ( !mail->status(EFlag_Sent) && !mail->unfinished()  )
                    count++;
                break;
            }
            case 3:
            {
                if ( mail->unfinished() )
                    count++;
                break;
            }
            default: break;
        }
    }

    return count;
}

EmailListItem* MailListView::getRef(QUuid id)
{
    for (int i = 0; i < rowCount(); ++i) {
        if ( ( (EmailListItem *) item(i, 0) )->id() == id)
            return (EmailListItem *) item(i, 0);
    }

    return NULL;
}

QString MailListView::currentMailbox()
{
    return _mailbox;
}

void MailListView::setCurrentMailbox(const QString &mailbox)
{
    _mailbox = mailbox;
}

/*  May be expanded to have a thread-view of messages.  Ignore all disabled
    code for now ( doesn't work yet either way )
*/
void MailListView::treeInsert(Email *mail)
{
    insertRow(0);
    setItem(0, 0, new EmailListItem(this, mail, 0) );
#ifdef QTOPIA4_TODO
    // For PDA need to make several EmailListItems, one for each column.
#endif
#ifdef QTOPIA_PHONE
    QSoftMenuBar::setLabel(this, Qt::Key_Select, rowCount() ? QSoftMenuBar::View : QSoftMenuBar::NoLabel);
#endif
/*
    if ( mail->inReplyTo().isEmpty() ) {
        new EmailListItem(this, mail);
        return;
    }

    QString mId = mail->inReplyTo();
    Email *m;
    EmailListItem *item = NULL;
    QListWidgetItemIterator it( this );
    for ( ; it.current(); ++it ) {
        m =  ( (EmailListItem *) it.current() )->getMail();
        if ( m->messageId() == mId ) {
            item = (EmailListItem *) it.current();
            break;
        }
    }

    if ( item ) {
        new EmailListItem(item, mail);
        item->setOpen(true);
    } else {
        new EmailListItem(this, mail);
    }
*/
    emit enableMessageActions( true );
}

void MailListView::clear()
{
    if (singleColumnMode()) {
        maxColumnWidth = QApplication::desktop()->availableGeometry().width();
        horizontalHeader()->resizeSection( 0, maxColumnWidth );
    }
    QTableWidget::clear();
    setRowCount( 0 );
    setColumnCount( columns.count() );
    setHorizontalHeaderLabels( columns );
#ifdef QTOPIA_PHONE
    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
#endif
    emit enableMessageActions( false );
}

void MailListView::ensureWidthSufficient( const QString &text )
{
    if (singleColumnMode()) {
        QFontMetrics fm( font() );
        int width = fm.width( text );
        if (width > maxColumnWidth) {
            horizontalHeader()->resizeSection( 0, width );
            maxColumnWidth = width;
        }
    }
}

bool MailListView::singleColumnMode()
{
    return mSingleColumnMode;
}

void MailListView::setSingleColumnMode( bool singleColumnMode )
{
    mSingleColumnMode = singleColumnMode;
}

void MailListView::readConfig( QSettings *conf )
{
    if (singleColumnMode()) {
        horizontalHeader()->resizeSection( 0,
                        QApplication::desktop()->availableGeometry().width() );
        horizontalHeader()->hide();
        defineSort(3, false);
    } else {
        int y;
        QString s;
        y = conf->value("mailidcount", -1).toInt();
        for (int x = 0; x < columnCount(); x++) {
            s.setNum(x);
            if ( (y = conf->value("querycol" + s, -1).toInt()) != -1) {
                horizontalHeader()->resizeSection( x, y );
            }
            if ( (y = conf->value("querycollabelpos" + s, -1).toInt()) != -1) {
                if ( y != x)
                    moveSection(x, y);
            }
        }
        int col = conf->value( "querycolsort", 3 ).toInt();
        bool ascend = conf->value( "querycolsortascending", false ).toBool();
        if (col != -1) {
            setByArrival( conf->value( "arrivaldate" ).toBool() );
            defineSort(col, ascend);
        }

        bool hide = conf->value("showheader", false ).toBool();
        if ( hide )
            horizontalHeader()->hide();
    }
}

void MailListView::writeConfig( QSettings *conf)
{
    QString temp;
    if (!singleColumnMode()) {
        for (int x = 0; x < columnCount(); x++) {
            temp.setNum(x);
            conf->setValue("querycol" + temp, columnWidth(x) );
            conf->setValue("querycollabelpos" + temp, labelPos(x) );
        }
        conf->setValue( "querycolsort", sortedColumn() );
        conf->setValue( "querycolsortascending", isAscending() );
        conf->setValue( "arrivaldate", arrivalDate() );
        conf->setValue( "showheader", horizontalHeader()->isHidden() );
    }
}

EmailListItem* MailListView::emailItemFromIndex( const QModelIndex & i ) const
{
    return (EmailListItem *)itemFromIndex( i );
}

// Keep the list of message headers scrolled hard to the left
void MailListView::scrollToLeft(int)
{
    // In right to left mode when the horizontal header is hidden a
    // phantom 70 pixel column appears. Scroll this out of the way.
    // It would be better to fix this elsewhere (in QTableWidget?)
    if (currentColumn() != 0)
        setCurrentCell( currentRow(), 0 );
    horizontalScrollBar()->setValue( 0 );
}

QContactModel *MailListView::contactModel()
{
    if (!mContactModel)
        mContactModel = new QContactModel( this );

    return mContactModel;
}
