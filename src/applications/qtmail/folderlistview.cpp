/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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


#include "folderlistview.h"

#include <qtopiaapplication.h>
#include <qmailtimestamp.h>

#include "searchview.h"
#include "accountlist.h"
#include "emailfolderlist.h"

#include <qmessagebox.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qdesktopwidget.h>
#include <qevent.h>
#include <QHeaderView>

static QPixmap* pm_normal = 0;
static QPixmap* pm_unread = 0;
static QPixmap* pm_unsent = 0;

static void ensurePixmaps()
{
    if ( !pm_normal ) {
	// These should be replaced once new icons are available
        pm_normal = new QPixmap(":image/flag_normal");
        pm_unread = new QPixmap(":image/flag_unread");
        pm_unsent = new QPixmap(":image/flag_tosend");
    }
}

FolderListItem::FolderListItem(QTreeWidget *parent, Folder *in, const QString& mailboxName)
    : QTreeWidgetItem( parent ),
      _folder(in),
      _highlight(false)
{
    init(mailboxName);
}

FolderListItem::FolderListItem(QTreeWidgetItem *parent, Folder *in, const QString& mailboxName)
    : QTreeWidgetItem(parent),
      _folder(in),
      _highlight(false)
{
    init(mailboxName);
}

void FolderListItem::init(QString name)
{
    if (name.isNull())
        name = _folder->mailbox();

    QIcon icon = MailboxList::mailboxIcon(name);

    setText(0, _folder->displayName() );
    int extent = qApp->style()->pixelMetric(QStyle::PM_SmallIconSize);
    setIcon( 0, icon.pixmap(extent));
}

Folder* FolderListItem::folder()
{
    return _folder;
}

void FolderListItem::setStatusText( const QString &str, bool highlight, IconType type )
{
    _statusText = str;
    _highlight = highlight;
    _type = type;
}

void FolderListItem::statusText( QString *str, bool *highlight, IconType *type )
{
    *str = _statusText;
    *highlight = _highlight;
    *type = _type;
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
      if ( static_cast<SystemFolder *>(_folder)->isSearch() )
        return ( "77" );

      QChar i = '7';
      QString s = _folder->mailbox();
      if ( s == MailboxList::InboxString)
        i = '1';
      else if ( s == MailboxList::OutboxString )
        i = '2';
      else if ( s == MailboxList::DraftsString )
        i = '3';
      else if ( s == MailboxList::SentString )
        i = '4';
      else if ( s == MailboxList::TrashString )
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

    FolderListItemDelegate *delegate = new FolderListItemDelegate( this );
    setItemDelegate( delegate );

    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(folderChanged(QTreeWidgetItem*)) );
    header()->hide();
    connect( this, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
             this, SLOT(itemClicked(QTreeWidgetItem*)) );
}

FolderListView::~FolderListView()
{
}


Folder* FolderListView::currentFolder() const
{
    FolderListItem *item = currentFolderItem();
    if ( item != NULL )
        return item->folder();

    return NULL;
}

bool FolderListView::setCurrentFolder(const Folder* folder)
{
    QModelIndex index = model()->index( 0, 0 );
    for ( ; index.isValid(); index = next( index ) ) {
        FolderListItem *item = folderItemFromIndex( index );
        if ( item->folder() == folder ) {
            setCurrentItem( item );
            return true;
        }
    }

    return false;
}

MailAccount* FolderListView::currentAccount() const
{
    FolderListItem *item = currentFolderItem();
    MailAccount *account = 0;

    while (item &&
           item->folder() &&
           item->folder()->folderType() != FolderTypeAccount)
        item = static_cast<FolderListItem*>(item->parent());

    if (item &&
        item->folder() &&
        item->folder()->folderType() == FolderTypeAccount)
        account = static_cast<MailAccount*>(item->folder());

    return account;
}

void FolderListView::keyPressEvent( QKeyEvent *e )
{
    switch( e->key() ) {
        case Qt::Key_Select:
        case Qt::Key_Space:
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            e->accept();
            emit viewMessageList();
        }
        break;

        case Qt::Key_Back:
        {
            e->accept();
            emit finished();
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

    //build inbox
    FolderListItem* inbox = static_cast<FolderListItem*>(topLevelItem(0));

    //build all other accounts
    QListIterator<MailAccount*> itAccount = list->accountIterator();
    while (itAccount.hasNext()) {
        MailAccount* accountInfo = itAccount.next();
        if (accountInfo->accountType() == MailAccount::POP ||
            accountInfo->accountType() == MailAccount::IMAP) {
            FolderListItem* accountItem = new FolderListItem(inbox, accountInfo, "");
            buildImapFolder(accountItem,accountInfo);
        }
    }
    setItemExpanded(inbox, true);
    setCurrentItem(inbox);
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
    FolderListItem *inbox_item = folderItemFromIndex( index );
    FolderListItem *item = static_cast<FolderListItem *>(inbox_item->child(0));
    Folder *folder;

    while (item != NULL) {
        folder = item->folder();
        if (folder->folderType() == FolderTypeAccount ) {
            if ( folder == account ) {
                bool selected = selectedChildOf(item);
                QString statusText;
                bool statusHighlight;
                IconType type;
                item->statusText( &statusText, &statusHighlight, &type );
                buildImapFolder(item,account); //rebuild imap subtree
                item->setStatusText( statusText, statusHighlight, type );

                if ( selected ) {
                    setCurrentItem( item );
                    setItemExpanded(item, true);
                }

                folderChanged( currentItem() );
                return;
            }
        }
        item = static_cast<FolderListItem *>(next( item ));
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
        folder = folderItemFromIndex( index )->folder();
        if ( folder->folderType() == FolderTypeAccount ) {

            //make updates
            if ( folder == account ) {
                EmailFolderList *inbox = _mailboxList->mailbox(MailboxList::InboxString);
                QMailIdList accountMessages = inbox->messagesFromAccount(account->id());
                foreach(QMailId id, accountMessages)
                    inbox->removeMail(id);
                
//                QListIterator<Email*> it2 = inbox->entryIterator();
//                while ( it2.hasNext() ) {
//                   Email *mail = it2.next();
//                    if ( mail->fromAccount() == account->id() ) {
//                        inbox->removeMail( mail->id(), true );
//                    }
                delete itemFromIndex( index );
                folderChanged( currentItem() );
                return;
            }
        }
    }
}

bool FolderListView::selectedChildOf(FolderListItem *folder)
{
   FolderListItem *selected = currentFolderItem();

   if (selected == NULL)
       return false;

    if (folder == selected)
        return true;

    while ( ( selected = static_cast<FolderListItem *>( selected->parent() ) ) != NULL ) {
        if (folder == selected)
            return true;
    }

    return false;
}

void FolderListView::changeToSystemFolder(const QString &str)
{
    Folder *folder;
    bool search = (str == MailboxList::LastSearchString);

    QModelIndex index = model()->index( 0, 0 ); //inbox
    for ( ; index.isValid(); index = next( index ) ) {
        FolderListItem *item = folderItemFromIndex( index );
        folder = item->folder();
        if ( folder->folderType() == FolderTypeSystem ) {
            if ( search && static_cast<SystemFolder *>(folder)->isSearch() ) {
                setCurrentItem( item );
                break;
            } else if ( folder->mailbox() == str ) {
                setCurrentItem( item );
                break;
            }
        }
    }
}

void FolderListView::updateFolderStatus(const QString &mailbox, const QString &txt, bool highlight, IconType type)
{
    Folder *folder;
    QModelIndex index = model()->index( 0, 0 );
    for ( ; index.isValid(); index = next( index ) ) {
        FolderListItem *item = folderItemFromIndex( index );
        folder = item->folder();
        if ( folder->folderType() == FolderTypeSystem ) {
            if ( static_cast<SystemFolder *>( folder )->mailbox() == mailbox ) {
                item->setStatusText( txt, highlight, type );
                dataChanged( index, index );
                return;
            }
        }
    }
}

void FolderListView::updateAccountStatus(const Folder *account, const QString &txt, bool highlight, IconType type)
{
    Folder *folder;
    QModelIndex index = model()->index( 0, 0 );
    for ( ; index.isValid(); index = next( index ) ) {
        FolderListItem *item = folderItemFromIndex( index );
        folder = item->folder();
        if ( folder == account ) {
            item->setStatusText( txt, highlight, type );
            dataChanged( index, index );
            return;
        }
    }
}

void FolderListView::folderChanged(QTreeWidgetItem *folder)
{
    QString str;

    if ( folder == NULL ) {
        emit folderSelected( reinterpret_cast<Folder *>( NULL ) );
        return;
    }

    emit folderSelected( static_cast<FolderListItem *>(folder)->folder() );
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
        if ( !box->isDeleted() && box->localCopy()) {
            subfolder = new FolderListItem(getParent(item, box->pathName(), box->getDelimiter()), box, box->displayName());
        }
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
        FolderListItem *item = folderItemFromIndex( index );
        if ( item->depth() <= level )
            return parent;              //failed

        folder = item->folder();
        if ( folder->folderType() != FolderTypeMailbox )
            return parent;              //failed

        if ( static_cast<Mailbox *>(folder)->pathName() == target )
            return item;
    }

    return parent;
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

FolderListItem *FolderListView::folderItemFromIndex( QModelIndex index ) const
{
    return static_cast<FolderListItem *>(itemFromIndex( index ));
}

FolderListItem *FolderListView::currentFolderItem() const
{
    return static_cast<FolderListItem *>(currentItem());
}

void FolderListView::restoreCurrentFolder()
{
    if (currentIndex.isValid()) {
	setCurrentItem( itemFromIndex( currentIndex ) );
	scrollToItem( itemFromIndex( currentIndex ) );
    }
}

void FolderListView::rememberCurrentFolder()
{
    currentIndex = indexFromItem( currentItem() );
}

/* Folder List Item Delegate */

FolderListItemDelegate::FolderListItemDelegate(FolderListView *parent)
    : QtopiaItemDelegate(parent),
      mParent(parent)
{
    ensurePixmaps();
}

void FolderListItemDelegate::paint(QPainter *painter,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    FolderListItem *item = mParent->folderItemFromIndex( index );
    bool statusHighlight;
    statusText = QString();
    if (item)
        item->statusText( &statusText, &statusHighlight, &type );

    QtopiaItemDelegate::paint(painter, option, index);
}

void FolderListItemDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
                     const QRect &rect, const QString &text) const
{
    QtopiaItemDelegate::drawDisplay(painter, option, rect, text);
    if (!statusText.isEmpty()) {
        QString str = statusText;
        if (option.direction == Qt::RightToLeft) {
            QString trim = statusText.trimmed();
            // swap new/total counts in rtl mode
            int sepPos = trim.indexOf( "/" );
            if (sepPos != -1) {
                str = trim.mid( sepPos + 1 );
                str += "/";
                str += trim.left( sepPos );
            }
        }
        QPixmap *pm = 0;
        if (type == UnreadMessages) {
            pm = pm_unread;
        } else if (type == UnsentMessages) {
            pm = pm_unsent;
        } else if (type == AllMessages) {
            pm = pm_normal;
        }
        QFontMetrics fontMetrics(option.font);
        int tw = fontMetrics.width(str);
        int margin = 5;
        int pw = margin;
        int ph = 0;
        if (pm) {
            pw = pm->width() + 2*margin;
            ph = pm->height();
        }
        QRect statusRect = option.direction == Qt::RightToLeft
            ? QRect(0, rect.top(), tw + pw, rect.height())
            : QRect(rect.left()+rect.width()-tw-pw, rect.top(), tw, rect.height());
        
        int v = (rect.height() - ph) / 2;
        QRect pixRect = option.direction == Qt::RightToLeft
            ? QRect(margin, rect.top(), pw-margin, v)
            : QRect(rect.left()+rect.width()-pw+margin, rect.top() + v, pw-2*margin, ph);
        painter->drawText(statusRect, Qt::AlignCenter, str);
        if (pm)
            painter->drawPixmap(pixRect, *pm);
    }
}
