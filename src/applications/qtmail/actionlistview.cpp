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

#include "actionlistview.h"
#include <QtopiaItemDelegate>
#include <QPainter>

class ActionListItem : public QListWidgetItem
{
public:
    ActionListItem(QListWidget *parent, const QIcon &icon, QString name, QString in);
    QString name();
    void setStatusText( const QString &str, IconType type );
    void statusText( QString *str, IconType *type );
    
private:
    QString mInternalName;
    QString _statusText;
    IconType _type;
};

ActionListItem::ActionListItem(QListWidget *parent, const QIcon &icon, QString name, QString in)
  : QListWidgetItem(icon, name, parent), mInternalName( in )
{
    _type = AllMessages;
}

QString ActionListItem::name()
{
    return mInternalName;
}

void ActionListItem::setStatusText( const QString &str, IconType type )
{
    _statusText = str;
    _type = type;
    listWidget()->update();
}

void ActionListItem::statusText( QString *str, IconType *type )
{
    *str = _statusText;
    *type = _type;
}


class ActionListItemDelegate : public QtopiaItemDelegate
{
public:
    ActionListItemDelegate(ActionListView *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                    const QModelIndex &index) const;
    void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
                     const QRect &rect, const QString &text) const;

private:
    ActionListView *mParent;
    mutable QString statusText;
    mutable IconType type;
};

/* Folder List Item Delegate */
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

ActionListItemDelegate::ActionListItemDelegate(ActionListView *parent)
    : QtopiaItemDelegate(parent),
      mParent(parent)
{
    ensurePixmaps();
}

void ActionListItemDelegate::paint(QPainter *painter,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    ActionListItem *item = static_cast<ActionListItem *>(mParent->actionItemFromIndex( index ));
    statusText = QString();
    if (item)
        item->statusText( &statusText, &type );

    QtopiaItemDelegate::paint(painter, option, index);
}

void ActionListItemDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
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

ActionListView::ActionListView(QWidget *parent)
    : QListWidget( parent )
{
    setObjectName( "actionView" );
    ActionListItemDelegate *delegate = new ActionListItemDelegate( this );
    setItemDelegate( delegate );
    mComposeItem = newItem( QT_TRANSLATE_NOOP("ActionListView","New message"),"ActionListView", true );
    mComposeItem->setIcon(QIcon(":icon/new"));
    mInboxItem = newItem( MailboxList::InboxString,"", false );
    mSentItem = newItem( MailboxList::SentString,"", false );
    mDraftsItem = newItem( MailboxList::DraftsString,"", false );
    mTrashItem = newItem( MailboxList::TrashString,"", false );
    mOutboxItem = newItem( MailboxList::OutboxString,"", false );
    mEmailItem = newItem( QT_TRANSLATE_NOOP("ActionListView","Email"),"ActionListView", true );
    setCurrentRow( 0 );
    connect( this, SIGNAL(itemActivated(QListWidgetItem*)),
             this, SLOT(itemSlot(QListWidgetItem*)) );
    connect( this, SIGNAL(currentRowChanged(int)),
             this, SLOT(currentFolderChanged(int)) );

    // Required to work around bug in Qt 4.3.2:
    setVerticalScrollMode(ScrollPerPixel);
}

ActionListView::~ActionListView()
{
}

// Simple convenience function
ActionListItem *ActionListView::newItem( const char *name, const char* context, bool useTr )
{
    QString displayName;
    if (useTr) // use tr
        displayName = qApp->translate( context, name );
    else // use mailboxTrName
        displayName = MailboxList::mailboxTrName( name );
    return new ActionListItem( this, MailboxList::mailboxIcon(name), displayName , name );
}

void ActionListView::itemSlot(QListWidgetItem *item)
{
    if (item == mComposeItem)
        emit composeMessage();
    else if (item == mEmailItem)
        emit emailSelected();
    else
        emit displayFolder( (static_cast<ActionListItem *>(item))->name() );
}

void ActionListView::currentFolderChanged(int row)
{
    ActionListItem *aItem = (static_cast<ActionListItem *>(item(row) ));
    if (aItem == mComposeItem)
        emit currentFolderChanged(QString::null);
    else if (aItem == mEmailItem)
        emit currentFolderChanged(QString::null);
    else
        emit currentFolderChanged( aItem->name() );
}

ActionListItem *ActionListView::folder(const QString &mailbox)
{
    for (int i = 1; i < count(); ++i) {
        ActionListItem *aItem = static_cast<ActionListItem *>(item( i ));
        if (mailbox == aItem->name()) {
            return aItem;
        }
    }

    return 0;
}

void ActionListView::updateFolderStatus(const QString &mailbox, const QString &txt, IconType type)
{
    if (ActionListItem *aItem = folder(mailbox)) {
        aItem->setStatusText( txt, type );
    }
}

void ActionListView::setCurrentFolder(const QString &mailbox)
{
    if (ActionListItem *aItem = folder(mailbox)) {
        setCurrentItem(aItem);
    }
}

ActionListItem *ActionListView::actionItemFromIndex( QModelIndex index ) const
{
    return static_cast<ActionListItem *>(itemFromIndex( index ));
}

