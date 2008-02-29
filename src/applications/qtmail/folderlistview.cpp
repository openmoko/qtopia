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


#include "folderlistview.h"

#include <qtopiaapplication.h>

#include "searchview.h"
#include "accountlist.h"
#include "emailfolderlist.h"
#include "common.h"

#include <qmessagebox.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qdesktopwidget.h>
#include <qevent.h>
#include <QHeaderView>

using namespace QtMail;

FolderListItem::FolderListItem(QTreeWidget *parent, Folder *in)
    : QTreeWidgetItem( parent )
{
    _folder = in;
    _statusText = "";
    _highlight = false;

    QString pixmap = "folder"; // No tr
    if ( _folder->folderType() == FolderTypeSearch ) {
        pixmap = "find"; // No tr
    } else if ( _folder->folderType() == FolderTypeSystem ) {
        if ( ((SystemFolder *) _folder)->systemType() == SystemTypeSearch )
            pixmap = "find"; // No tr
    }

    setText(0, _folder->displayName() );
    int extent = qApp->style()->pixelMetric(QStyle::PM_SmallIconSize);
    setIcon( 0, QIcon( ":icon/"+pixmap ).pixmap(extent));
}

FolderListItem::FolderListItem(QTreeWidgetItem *parent, Folder *in)
    : QTreeWidgetItem(parent)
{
    _folder = in;
    _statusText = "";
    _highlight = false;

    QString pixmap = "folder"; // No tr
    if ( _folder->folderType() == FolderTypeSearch ) {
        pixmap = "find"; // No tr
    } else if ( _folder->folderType() == FolderTypeSystem ) {
        if ( ((SystemFolder *) _folder)->systemType() == SystemTypeSearch )
            pixmap = "find"; // No tr
    }

    setText(0, _folder->displayName() );
    int extent = qApp->style()->pixelMetric(QStyle::PM_SmallIconSize);
    setIcon( 0, QIcon( ":icon/"+pixmap ).pixmap(extent));
}

Folder* FolderListItem::folder()
{
    return _folder;
}

void FolderListItem::setStatusText( const QString &str, bool highlight, const QColor &col )
{
    _statusText = str;
    _highlight = highlight;
    _col = col;
}

void FolderListItem::statusText( QString *str, bool *highlight, QColor *col )
{
    *str = _statusText;
    *highlight = _highlight;
    *col = _col;
}

int FolderListItem::depth()
{
    int count = 0;
    QTreeWidgetItem *item = this;
    while (item->parent()) {
        ++count;
        item = item->parent();
    }
    return count;
}

#ifdef QTOPIA4_TODO
void FolderListItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment )
{
    QColorGroup _cg( cg );

    QString t = text( column );

    QTreeWidget *lv = listView();
    int r = lv ? lv->itemMargin() : 1;
    const QPixmap * icon = pixmap( column );

    p->fillRect( 0, 0, width, height(), cg.brush( QColorGroup::Base ) );

    int marg = r;
    if ( isSelected() &&
         (column==0 || listView()->allColumnsShowFocus()) ) {
        p->fillRect( r - marg, 0, width - r + marg, height(),
                     cg.brush( QColorGroup::Highlight ) );
        p->setPen( cg.highlightedText() );
    } else {
        p->setPen( cg.text() );
    }

    if ( icon ) {
        p->drawPixmap( r, (height()-icon->height())/2, *icon );
        r += icon->width() + listView()->itemMargin();
    }

    if ( !t.isEmpty() ) {
        if ( ! (alignment & AlignTop | alignment & AlignBottom) )
            alignment |= AlignVCenter;


        if ( !_statusText.isEmpty() ) {
            if ( _highlight ) {
                QFont font = p->font();
                font.setBold( true );
                p->setFont( font );
            }

            QRect rBound;

            p->drawText( r, 0, width-marg-r, height(), alignment, t, -1, &rBound );
            r += rBound.width();

            if ( !isSelected() && _highlight ) {
                _cg.setColor( QColorGroup::Text, _col );
                p->setPen( _cg.text() );
            }

            p->drawText( r, 0, width-marg-r, height(), alignment, _statusText);
        } else {
            p->drawText( r, 0, width-marg-r, height(), alignment, t );
        }

    }

    _cg.setColor( QColorGroup::Text, column );
}

int FolderListItem::width( const QFontMetrics& fm,  const QTreeWidget* lv, int c ) const
{
    int w;
    if ( !_statusText.isEmpty() ) {
        QFont font = lv->font();
        font.setBold( _highlight );
        QFontMetrics fm2( font );

        w = fm2.width( text( c ) + _statusText ) + lv->itemMargin() * 2;
    } else {
        w = fm.width( text( c ) ) + lv->itemMargin() * 2;
    }

    const QPixmap * pm = pixmap( c );
    if ( pm )
        w += pm->width() + lv->itemMargin(); // ### correct margin stuff?

    return qMax( w, QApplication::globalStrut().width() );
}
#endif

/*  We want a particular layout, use sort to enforce it  */
QString FolderListItem::key(int c, bool) const
{
  if ( c != 0 )
    return QString();

  int type = _folder->folderType();
  switch( type ) {
    case FolderTypeSystem:
    {
        // system search folder(last search) has a mailbox equal to one of the other real mailboxes,
        // but we still want it to appear last though.
      if ( ((SystemFolder *) _folder)->isSearch() )
        return ( "77" );

      QChar i = '7';
      QString s = _folder->mailbox();
      if ( s == InboxString)
        i = '1';
      else if ( s == OutboxString )
        i = '2';
      else if ( s == DraftsString )
        i = '3';
      else if ( s == SentString )
        i = '4';
      else if ( s == TrashString )
        i = '5';

      return QString::number( type ) + i;
    }
    case FolderTypeAccount:
    {
      return "2" + text(0);
    }
    case FolderTypeMailbox:
    {
      return "3" + text(0);
    }
    default:    //folderTypeSearch
    {
      return "4" + text(0);
    }
  }
}

/* Folder list view  */

FolderListView::FolderListView(MailboxList *list, QWidget *parent, const char *name)
    : QTreeWidget( parent )
{
    setObjectName( name );
    setFrameStyle( NoFrame );
    sortItems(0, Qt::AscendingOrder );

    _mailboxList = list;

    searchFolder = new SystemFolder(SystemTypeSearch, LastSearchString);

    FolderListItemDelegate *delegate = new FolderListItemDelegate( this );
    setItemDelegate( delegate );

    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(folderChanged(QTreeWidgetItem*)) );
    header()->hide();
    connect( &menuTimer, SIGNAL(timeout()), SLOT(showFolderMenu()) );
    connect( this, SIGNAL(itemSelectionChanged()), SLOT(cancelMenuTimer()) );
#ifdef QTOPIA_PHONE
    connect( this, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
             this, SLOT(itemClicked(QTreeWidgetItem*)) );
#endif
}

FolderListView::~FolderListView()
{
}


Folder* FolderListView::currentFolder()
{
    FolderListItem *item = (FolderListItem *) currentItem();
    if ( item != NULL )
        return item->folder();

    return NULL;
}

MailAccount* FolderListView::currentAccount()
{
    FolderListItem *item = (FolderListItem*)currentItem();
    MailAccount *account = 0;

    while (item &&
           item->folder() &&
           item->folder()->folderType() != FolderTypeAccount)
        item = (FolderListItem*)item->parent();

    if (item &&
        item->folder() &&
        item->folder()->folderType() == FolderTypeAccount)
        account = (MailAccount*)item->folder();

    return account;
}

void FolderListView::keyPressEvent( QKeyEvent *e )
{
    switch( e->key() ) {
#ifdef QTOPIA_PHONE
        case Qt::Key_Select:
#endif
        case Qt::Key_Space:
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            emit viewMessageList();
        }
        break;
        default:  QTreeWidget::keyPressEvent( e );
    }
}

QSize FolderListView::sizeHint() const
{
    return QSize(-1,-1);
}

QSize FolderListView::minimumSizeHint() const
{
    return QSize(-1,-1);
}

void FolderListView::setupFolders(AccountList *list)
{
    clear();
    //build system folders
    QStringList mboxList = _mailboxList->mailboxes();
    for (QStringList::Iterator it = mboxList.begin(); it != mboxList.end(); ++it) {
        new FolderListItem(this, new SystemFolder(SystemTypeMailbox, *it ) );
    }
    new FolderListItem(this, searchFolder);
    //build inbox
    QListIterator<MailAccount*> itAccount = list->accountIterator();
    FolderListItem* inbox = (FolderListItem*)topLevelItem(0);
    //build all other accounts
    while (itAccount.hasNext()) {
        MailAccount* accountInfo = itAccount.next();
        FolderListItem* accountItem = new FolderListItem(inbox, accountInfo);
        buildImapFolder(accountItem,accountInfo);
    }
    setItemExpanded(inbox, true);
}

QModelIndex FolderListView::next(QModelIndex mi, bool nextParent)
{
    if (mi.child(0,0).isValid() && !nextParent)
      return mi.child(0,0);
    if (mi.sibling( mi.row() + 1, mi.column() ).isValid())
        return mi.sibling( mi.row() + 1, mi.column() );
    if (mi.parent().isValid())
         return next( mi.parent(),true);
    return QModelIndex(); // invalid e.g. correct
}

QTreeWidgetItem* FolderListView::next(QTreeWidgetItem *item)
{
  QModelIndex m = indexFromItem(item);
  QModelIndex nextIndex = next(m);
  if(!nextIndex.isValid())
    return 0;
  else
    return itemFromIndex(nextIndex);
}

// receives an account, finds it in the listview and makes
// any necessary updates
void FolderListView::updateAccountFolder(MailAccount *account)
{
    QModelIndex index = model()->index( 0, 0 ); //inbox
    FolderListItem *inbox_item = (FolderListItem *)itemFromIndex( index );
    FolderListItem *item = (FolderListItem *) inbox_item->child(0);
    Folder *folder;

    while (item != NULL) {
        folder = item->folder();
        if (folder->folderType() == FolderTypeAccount ) {
            if ( folder == account ) {
                bool selected = selectedChildOf(item);
                QString statusText;
                bool statusHighlight;
                QColor statusCol;
                item->statusText( &statusText, &statusHighlight, &statusCol );
        buildImapFolder(item,account); //rebuild imap subtree
                item->setStatusText( statusText, statusHighlight, statusCol );

                if ( selected ) {
                    setCurrentItem( item );
                    setItemExpanded(item, true);
                }

                folderChanged( currentItem() );
                return;
            }
        }
        item = (FolderListItem *)next( item );
    }

    //if we get here, it was a new folder
    item = new FolderListItem(inbox_item,account);
    buildImapFolder(item,account);
}

void FolderListView::deleteAccountFolder(MailAccount *account)
{
    Folder *folder;

    QModelIndex rootindex = model()->index( 0, 0 );
    QModelIndex index = next( rootindex);
    if(index == rootindex)
      return;
    for ( ; index.isValid(); index = next( index ) ) {
        folder = ( (FolderListItem *) itemFromIndex( index ) )->folder();
        if ( folder->folderType() == FolderTypeAccount ) {

            //make updates
            if ( folder == account ) {
                EmailFolderList *inbox = _mailboxList->mailbox(InboxString);
                QListIterator<Email*> it2 = inbox->entryIterator();
                while ( it2.hasNext() ) {
                    Email *mail = it2.next();
                    if ( mail->fromAccount() == account->id() ) {
                        inbox->removeMail( mail->uuid(), true );
                    }
                }
                delete itemFromIndex( index );
                folderChanged( currentItem() );
                return;
            }
        }
    }
}

bool FolderListView::selectedChildOf(FolderListItem *folder)
{
   FolderListItem *selected = (FolderListItem *) currentItem();

   if (selected == NULL)
       return false;

    if (folder == selected)
        return true;

    while ( ( selected = (FolderListItem *) selected->parent() ) != NULL ) {
        if (folder == selected)
            return true;
    }

    return false;
}

void FolderListView::changeToSystemFolder(const QString &str)
{
    Folder *folder;
    bool search = (str == LastSearchString);

    QModelIndex index = model()->index( 0, 0 ); //inbox
    for ( ; index.isValid(); index = next( index ) ) {
        FolderListItem *item = (FolderListItem *) itemFromIndex( index );
        folder = item->folder();
        if ( folder->folderType() == FolderTypeSystem ) {
            if ( search && ((SystemFolder *) folder)->isSearch() ) {
                setCurrentItem( item );
                break;
            } else if ( folder->mailbox() == str ) {
                setCurrentItem( item );
                break;
            }
        }
    }
}

void FolderListView::updateFolderStatus(const QString &mailbox, const QString &txt, bool highlight, const QColor &col)
{
    Folder *folder;
    QModelIndex index = model()->index( 0, 0 );
    for ( ; index.isValid(); index = next( index ) ) {
        FolderListItem *item = (FolderListItem *) itemFromIndex( index );
        folder = item->folder();
        if ( folder->folderType() == FolderTypeSystem ) {
            if ( ( (SystemFolder *) folder )->mailbox() == mailbox ) {
                item->setStatusText( txt, highlight, col );
                dataChanged( index, index );
                return;
            }
        }
    }
}

void FolderListView::updateAccountStatus(const Folder *account, const QString &txt, bool highlight, const QColor &col)
{
    Folder *folder;
    QModelIndex index = model()->index( 0, 0 );
    for ( ; index.isValid(); index = next( index ) ) {
        FolderListItem *item = (FolderListItem *) itemFromIndex( index );
        folder = item->folder();
        if ( folder == account ) {
            item->setStatusText( txt, highlight, col );
            dataChanged( index, index );
            return;
        }
    }
}

void FolderListView::setLastSearch(Search *search)
{
    searchFolder->setSearch( search );
}

Search* FolderListView::lastSearch()
{
    return searchFolder->search();
}

void FolderListView::folderChanged(QTreeWidgetItem *folder)
{
    QString str;

    if ( folder == NULL ) {
        showFolderChoice( NULL );
        emit folderSelected( (Folder *) NULL );
        return;
    }

    showFolderChoice( (FolderListItem *) folder);
    emit folderSelected( ((FolderListItem *) folder)->folder() );
}

void FolderListView::buildImapFolder(FolderListItem *item, MailAccount* account)
{
    Q_ASSERT(item);
    Q_ASSERT(account);
    Mailbox *box = 0;
    FolderListItem* subfolder = 0;

    //delete current subtree
    blockSignals(true);
    QList<QTreeWidgetItem*> children = item->takeChildren();
    while (!children.isEmpty())
        delete children.takeFirst();
    blockSignals(false);



    //build new subtree
    QListIterator<Mailbox*> it(account->mailboxes);
    while ( it.hasNext()) {
    box = it.next();
        if ( !box->isDeleted() && box->localCopy())
            subfolder = new FolderListItem(getParent(item, box->pathName(), box->getDelimiter() ), (Folder *) box);
    }
}

/*  Returns the parent mailbox/account in the treewidget of the given account.
        All mailboxes are sorted by name before added, so the conversion from
        flat to tree should not produce inccorect results.
*/
FolderListItem* FolderListView::getParent(FolderListItem *parent, QString name, QString delimiter)
{
    QStringList list = name.split(delimiter);
    list.removeAll( list.last() );

    if (list.count() == 0)
        return parent;

    QString target = list.join(delimiter);

    int level = parent->depth();
    Folder *folder;
    QModelIndex index = indexFromItem(parent); //parent index
    index = next(index); //get first child

    for ( ; index.isValid(); index = next( index ) ) {
        FolderListItem *item = (FolderListItem *) itemFromIndex( index );
        if ( item->depth() <= level )
            return parent;              //failed

        folder = item->folder();
        if ( folder->folderType() != FolderTypeMailbox )
            return parent;              //failed

        if ( ((Mailbox *) folder)->pathName() == target )
            return item;
    }

    return parent;
}

void FolderListView::cancelMenuTimer()
{
    if( menuTimer.isActive() )
        menuTimer.stop();
}

void FolderListView::itemClicked(QTreeWidgetItem *i)
{
    if (i)
        emit viewMessageList();
}

void FolderListView::popFolderSelected(int value)
{
    QModelIndex index = model()->index( 0, 0 );
    while (value) {
        index = next( index );
        --value;
    }
    setCurrentItem( itemFromIndex( index ) );
    scrollToItem( itemFromIndex( index ) );
}

QMenu* FolderListView::folderParentMenu(QMenuBar *host)
{
    folderBar = new QMenu(host);
    newQueryAction = folderBar->addAction( QIcon(":icon/find"), tr("New query"),
                                           this, SLOT( newQuery() ) );
    modifyQueryAction = folderBar->addAction( QIcon(":icon/edit"), tr("Modify query"),
                                              this, SLOT( modifyQuery() ) );
    deleteFolderAction = folderBar->addAction( QIcon(":icon/trash"), tr("Delete query"),
                                               this, SLOT( deleteFolder() ) );
    showFolderChoice( NULL );

    return folderBar;
}

/* sets the valid choices for the parent folder menu.  Basically
   the same as showfoldermenu()  (might join)  */
void FolderListView::showFolderChoice(FolderListItem *item)
{
    // mark all as not active
    newQueryAction->setVisible( false );
    modifyQueryAction->setVisible( false );
    deleteFolderAction->setVisible( false );

    if ( item == NULL )
        return;

    Folder *folder = item->folder();

    if ( folder->folderType() == FolderTypeSystem ) {
        if ( ((SystemFolder *) folder)->systemType() != SystemTypeSearch )
            newQueryAction->setVisible( true );
    } else if ( folder->folderType() == FolderTypeSearch ) {
        modifyQueryAction->setVisible( true );
        deleteFolderAction->setVisible( true );
    }
}

void FolderListView::showFolderMenu()
{
    FolderListItem *item = (FolderListItem *) currentItem();

    if ( item == NULL )
        return;

    Folder *folder = item->folder();
    QMenu *popFolder = new QMenu(this);
    bool empty = false;

    if ( folder->folderType() == FolderTypeSystem ) {
        if ( !(folder->mailbox() == TrashString )  && !((SystemFolder *) folder)->isSearch() ) {
            popFolder->addAction( QIcon(":icon/find"), tr( "New query" ), this, SLOT( newQuery() ) );
        } else if ( folder->mailbox() == TrashString && ! ((SystemFolder *) folder)->isSearch() ) {
            popFolder->addAction( QIcon(":icon/trash"), tr( "Empty trash" ), this, SIGNAL( emptyFolder() ) );
        } else {
            empty = true;
        }
    } else if ( folder->folderType() == FolderTypeSearch ) {
        popFolder->addAction( QIcon(":icon/edit"), tr("Modify query"), this, SLOT( modifyQuery() ) );
        popFolder->addSeparator();
        popFolder->addAction( QIcon(":icon/trash"), tr("Delete query"), this, SLOT( deleteFolder() ) );
    } else {
        empty = true;
    }

    if ( !empty )
        popFolder->popup( QCursor::pos() );

}

void FolderListView::mousePressEvent( QMouseEvent * e )
{
    QTreeWidget::mousePressEvent( e );
    menuTimer.setSingleShot( true );
    menuTimer.start( 500 );
}

void FolderListView::mouseReleaseEvent( QMouseEvent * e )
{
    QTreeWidget::mouseReleaseEvent( e );
    menuTimer.stop();
}

void FolderListView::newQuery()
{
    SearchView searchView(true, this,Qt::Dialog); // No tr
    searchView.setWindowTitle( tr("New query") );
    searchView.setModal(true);

    FolderListItem *item = (FolderListItem *) currentItem();
    Folder *folder = item->folder();
    searchView.setQueryBox( folder->mailbox() );

    QtopiaApplication::execDialog(&searchView);
    if (searchView.result() == QDialog::Accepted) {
        Search *newSearch = searchView.getSearch();

        Folder *newFolder = new SearchFolder(newSearch);
        new FolderListItem(item, newFolder);
    }
}

void FolderListView::modifyQuery()
{
    FolderListItem *item = (FolderListItem *) currentItem();
    SearchView searchView(true, this,Qt::Dialog); // No tr
    searchView.setWindowTitle( tr("Modify query") );

    Folder *folder = item->folder();
    if (folder->folderType() != FolderTypeSearch) {
        qWarning("not a virtual folder");
        return;
    }

    searchView.setQueryBox( folder->mailbox() );
    searchView.setSearch( ( (SearchFolder *) folder )->search() );

    QtopiaApplication::execDialog(&searchView);
    if (searchView.result() == QDialog::Accepted) {
        Search *newSearch = searchView.getSearch();
        delete ( ( (SearchFolder *) folder )->search() );
        ( (SearchFolder *) folder )->setSearch(newSearch);
        item->setText(0, newSearch->name() );

        searchView.hide();      // in case it's a slow search
        emit folderSelected( folder );
    }
}

void FolderListView::deleteFolder()
{
    FolderListItem *item = (FolderListItem *) currentItem();

    if ( item == NULL )
        return;

    Folder *folder = item->folder();

    if ( folder->folderType() == FolderTypeSearch ) {
        delete folder;
        delete item;

        folderChanged( currentItem() );
    }

}

void FolderListView::saveQueries(QString mainGroup, QSettings *config)
{
    FolderListItem *item;
    int count = 0, posCount = 0;

    QModelIndex index = model()->index( 0, 0);
    for ( ; index.isValid(); index = next( index ) ) {
        item = (FolderListItem *) itemFromIndex( index );

        if ( item == (FolderListItem *) currentItem() ) {
            config->beginGroup( mainGroup );
            config->setValue("folderselected", posCount);
            config->endGroup();
        }

        Folder *folder = item->folder();
        if ( folder->folderType() == FolderTypeSearch ) {
            config->beginGroup( "query_" + QString::number(count) );
            saveSingleQuery(config, folder);
            count++;
            config->endGroup();
        }

        posCount++; //current position in QTreeWidget
    }

    config->beginGroup("lastsearch");
    saveSingleQuery(config, searchFolder);
    config->endGroup();

    // save count
    config->beginGroup(mainGroup);
    config->setValue("querycount", count );
    config->endGroup();
}

void FolderListView::saveSingleQuery(QSettings *config, Folder *folder)
{
    Search *search = NULL;

    if ( folder->folderType() == FolderTypeSearch ) {
        search = ( (SearchFolder * ) folder )->search();
    } else if ( folder->folderType() == FolderTypeSystem ) {
        if ( ((SystemFolder *) folder)->systemType() == SystemTypeSearch ) {
            search = ((SystemFolder *) folder)->search();
        }
    }

    if (search == NULL) {
        qWarning("trying to save invalid folder");
        return;
    }

    config->setValue("mailbox", search->mailbox() );
    config->setValue("name", search->name() );
    config->setValue("from", search->getFrom() );
    config->setValue("to", search->getTo() );
    config->setValue("subject", search->getSubject() );
    config->setValue("body", search->getBody() );
    config->setValue("status", (int) search->status() );
    if ( !search->getBeforeDate().isNull() ) {
        config->setValue("datebefore", search->getBeforeDate().toString() );
    } else {
        config->setValue("datebefore", "" );
    }
    if ( !search->getAfterDate().isNull() ) {
        config->setValue("dateafter", search->getAfterDate().toString() );
    } else {
        config->setValue("dateafter", "" );
    }
}

void FolderListView::readQueries(QString mainGroup, QSettings *config)
{
    SearchFolder  *sFolder;
    Search *search;

    config->beginGroup(mainGroup);
    int count = config->value("querycount", 0).toInt();

    for (int x = 0; x < count; x++) {
        config->beginGroup( "query_" + QString::number(x) );
        search = readSingleQuery(config);
        sFolder = new SearchFolder(search);

        Folder *folder;
        FolderListItem *item;
        QModelIndex index = model()->index( 0, 0 );
        for ( ; index.isValid(); index = next( index ) ) {
            item = (FolderListItem *) itemFromIndex( index );
            folder = item->folder();
            if ( folder->folderType() == FolderTypeSystem ) {
                if ( ( (SystemFolder *) folder )->mailbox() == search->mailbox() ) {
                    new FolderListItem( item, sFolder);
                    break;
                }
            }
        }
        config->endGroup();
    }
    config->endGroup();

    config->beginGroup("lastsearch");
    search = readSingleQuery(config);
    searchFolder->setSearch( search );
    config->endGroup();

    config->beginGroup(mainGroup);
    count = config->value("folderselected", -1).toInt();
    if ( count != -1 ) {
        QModelIndex index = model()->index( 0, 0 );
        while (count) {
            index = next( index );
            --count;
        }
        if (index.isValid()) {
            scrollToItem( itemFromIndex( index ) );
            setCurrentIndex( index );
        }
    }
    config->endGroup();


}

Search* FolderListView::readSingleQuery(QSettings *config)
{
    Search *search = new Search();

    search->setMailbox( config->value("mailbox", "inbox").toString() );
    search->setMailFrom( config->value("from").toString().trimmed() );
    search->setMailTo( config->value("to").toString().trimmed() );
    search->setMailSubject( config->value("subject").toString().trimmed() );
    search->setMailBody( config->value("body").toString().trimmed() );
    search->setStatus( config->value("status", (int) Search::Any).toInt() );
    search->setName( config->value("name").toString().trimmed() );

    QString strDate = config->value("dateafter").toString();
    if (!strDate.isEmpty())
        search->setAfterDate( Email::parseDate(strDate).date() );
    strDate = config->value("datebefore").toString();
    if (!strDate.isEmpty())
        search->setBeforeDate( Email::parseDate(strDate).date() );

    return search;
}

FolderListItem *FolderListView::folderItemFromIndex( QModelIndex index )
{
    return (FolderListItem *)itemFromIndex( index );
}

/* Folder List Item Delegate */

FolderListItemDelegate::FolderListItemDelegate(FolderListView *parent)
    : QItemDelegate(parent),
      mParent(parent)
{
}

void FolderListItemDelegate::paint(QPainter *painter,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());
    const QAbstractItemModel *model = index.model();
    Q_ASSERT(model);

    FolderListItem *item = mParent->folderItemFromIndex( index );
    QString statusText;
    bool statusHighlight;
    QColor statusCol;
    if ( item )
        item->statusText( &statusText, &statusHighlight, &statusCol );

    QStyleOptionViewItem opt = option;

    // Set color group
    opt.palette.setCurrentColorGroup(option.state & QStyle::State_Enabled
                                     ? QPalette::Active : QPalette::Disabled);

    // set font
    QVariant value = model->data(index, Qt::FontRole);
    if (value.isValid())
        opt.font = qvariant_cast<QFont>(value);

    // set text alignment
    value = model->data(index, Qt::TextAlignmentRole);
    if (value.isValid())
        opt.displayAlignment = QFlag(value.toInt());

    // set text color
    value = model->data(index, Qt::TextColorRole);
    if (value.isValid() && qvariant_cast<QColor>(value).isValid())
        opt.palette.setColor(QPalette::Text, qvariant_cast<QColor>(value));

    // do layout

    // decoration
    value = model->data(index, Qt::DecorationRole);
    QPixmap pixmap = decoration(opt, value);
    QRect pixmapRect = (pixmap.isNull() ? QRect(0, 0, 0, 0)
                        : QRect(QPoint(0, 0), option.decorationSize));

    // display
    QRect textRect;
    QString text = model->data(index, Qt::DisplayRole).toString().simplified();
    QString orgText = text;
    QRect statusRect;
    const int elideMargin = 6;
    if (item && !statusText.isEmpty()) {
        QFontMetrics fontMetrics(opt.font);
        statusRect = QRect(0, 0, fontMetrics.width(statusText),
                           fontMetrics.lineSpacing());
        text += statusText;
    }
    if (!text.isEmpty())
    {
        QFontMetrics fontMetrics(opt.font);
        textRect = QRect(0, 0, fontMetrics.width(text) + elideMargin,
                         fontMetrics.lineSpacing());
    }

    // check
    value = model->data(index, Qt::CheckStateRole);
    QRect checkRect = check(opt, opt.rect, value);
    Qt::CheckState checkState = static_cast<Qt::CheckState>(value.toInt());

    QRect savedRect = opt.rect;
    opt.rect.setWidth( pixmapRect.width() + textRect.width() + elideMargin );
    doLayout(opt, &checkRect, &pixmapRect, &textRect, false);
    opt.rect = savedRect;

    statusRect.translate( textRect.topRight() );
    statusRect.setWidth( statusRect.width() + 2 );
    statusRect.setLeft( textRect.right() - statusRect.width() );

    // handle rtl drawing
    if (opt.direction == Qt::RightToLeft) {
        checkRect.setRect( opt.rect.left() + opt.rect.width() - checkRect.width(), opt.rect.top(),
                            checkRect.width(), checkRect.height() );
        pixmapRect.setRect( opt.rect.left() + opt.rect.width() - pixmapRect.width() - 3, opt.rect.top(),
                            pixmapRect.width(), pixmapRect.height() );
        textRect.setRect( pixmapRect.left() - textRect.width(), textRect.top(),
                            textRect.width(), textRect.height() );
        statusRect.setRect( textRect.left() - statusRect.width(), statusRect.top(),
                            statusRect.width(), statusRect.height() );
    }

    // draw the background color
    if (option.showDecorationSelected && (option.state & QStyle::State_Selected)) {
        QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? QPalette::Normal : QPalette::Disabled;
        painter->fillRect(option.rect, option.palette.brush(cg, QPalette::Highlight));
    } else {
        value = model->data(index, Qt::BackgroundColorRole);
        if (value.isValid() && qvariant_cast<QColor>(value).isValid())
            painter->fillRect(option.rect, qvariant_cast<QColor>(value));
    }

    // draw the item
    if (checkRect.isValid())
        drawCheck(painter, opt, checkRect, checkState);
    if (pixmapRect.isValid())
        drawDecoration(painter, opt, pixmapRect, pixmap);

    // avoid drawing highlight color twice.
    opt.palette.setColor(QPalette::Highlight, QColor(0,0,0,0));

    if (!orgText.isEmpty()) {
        drawDisplay(painter, opt, textRect, orgText);
        drawFocus(painter, opt, textRect);
    }
    if (item && !statusText.isEmpty()) {
        opt.palette.setColor( QPalette::Text, statusCol );
        opt.font.setBold( statusHighlight );
        drawDisplay(painter, opt, statusRect, statusText);
        drawFocus(painter, opt, statusRect);
    }
}

