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

#include "bookmarkdelegate.h"

#include "bookmark.h"

#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QVariant>
#include <QPixmap>


/*!
  \internal
  \class BookmarkDelegate
  \brief Renderer for a list of Bookmark objects.

  \mainclass
  BookmarkDelegate can display items in two different modes - Navigation mode
  (displayMode = true) and Reorganise mode (displayMode = false). In Reorganise
  mode, the currently selected item is displayed with an icon of the given name.

  \sa BookmarkList
  \sa Bookmark
*/


/*! \fn BookmarkDelegate::BookmarkDelegate(QObject *parent = 0)
  Initializes the delegate, creating a QIcon object.
*/
BookmarkDelegate::BookmarkDelegate(QObject *parent)
    : QItemDelegate(parent)
    , icon(":icon/shuffle")
    , displayMode(true)
{
}

/*! \fn void BookmarkDelegate::setDisplayMode(bool displayMode)
  Updates the current display mode. If _displayMode is true, the delegate is now
  in Navigation mode. If false, the delegate is now in Reorganise mode.
*/
void BookmarkDelegate::setDisplayMode(bool _displayMode)
{
    displayMode = _displayMode;
}

/*! \fn void BookmarkDelegate::paint(QPainter *painter,const QStyleOptionViewItem &option, const QModelIndex &index) const
  Overrides superclass functionality to draw the item at the given index.
*/
void BookmarkDelegate::paint(QPainter *painter,const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // Probably not necessary for this implementation, but future proofs it.
    painter->save();

    // Ditto.
    QStyleOptionViewItem newOption = option;

    QVariant v = index.model()->data(index);
    if ( v.canConvert<Bookmark>() ) {
        // Retrieve the item at this index.
        Bookmark dataItem = v.value<Bookmark>();

        // Draw the text.
        QString text = dataItem.getTitle();
        drawDisplay(painter,newOption,newOption.rect,text);

        // Draw features of selected item.
        if ( option.state & QStyle::State_Selected ) {
            // Draw the icon -- but only if we're in Reorganise mode.
            if ( !displayMode ) {
                QSize sz(newOption.rect.width(),newOption.rect.height());
                sz = icon.actualSize(sz);
                QPixmap pixmap = icon.pixmap(sz);
                newOption.decorationAlignment = Qt::AlignRight;
                drawDecoration(painter,newOption,newOption.rect,pixmap);
            }

            newOption = option;
            drawFocus(painter,newOption,newOption.rect);
        }
    } else {
        qWarning("HelpBrowser/BookmarkDelegate: Not possible to convert the QVariant item into a Bookmark.");
    }

    painter->restore();
}
