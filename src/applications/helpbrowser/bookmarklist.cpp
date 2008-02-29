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

#include "bookmarklist.h"
#include "bookmarkmodel.h"
#include "bookmarkdelegate.h"

#include <QKeyEvent>
#include <QModelIndex>

#include <QDebug>

/*!
  \internal
  \class BookmarkList
  \brief Displays a list of Bookmark objects.

  \mainclass
  Allows Bookmark objects to be added, or else removed via the model (BookmarkModel).
  Bookmarks are displayed in two modes: Navigation mode allows the user to navigate
  Bookmarks, delete and select. Reorganise mode allows the user to rearrange the items,
  in addition to delete and select.

  \sa BookmarksUI
  \sa BookmarkModel
  \sa BookmarkDelegate
  \sa Bookmark
*/


/*! \fn BookmarkList::BookmarkList(QWidget *parent)
  Initializes the UI with a specific data model and a specific renderer.
  The UI always initializes in Navigation mode (displayMode = true).

*/
BookmarkList::BookmarkList(QWidget *parent)
    : QListView(parent)
    , displayMode(true)
{
    init();
}

/*! \fn BookmarkList::~BookmarkList()
*/
BookmarkList::~BookmarkList()
{
    delete delegate;
    delete bookmarkModel;
}

// Called by ctor to build the object.
void BookmarkList::init()
{
    bookmarkModel = new BookmarkModel(this);
    setModel(bookmarkModel);

    delegate = new BookmarkDelegate(this);
    setItemDelegateForColumn(0,delegate);

    // Initialise delegate's mode.
    delegate->setDisplayMode(displayMode);
}

/*! \fn void BookmarkList::bookmarkSelected(Bookmark)
  Signal to indicate that the user has chosen a specific Bookmark for display
  i.e. by activating the Select (OK) button. Note that it is the responsibility
  of external classes to display the Url, if desired.
*/

/*! \fn void BookmarkList::displayModeChanged(bool displayMode)
  Signal to indicate that the current display mode has changed. If displayMode
  is true, the list has changed to Display/Navigate mode. If displayMode is
  false, the list has changed to Shift/Reorganise mode.
*/

/*! \fn BookmarkModel *BookmarkList::getBookmarkModel() const
   Convenience method to retrieve the appropriate data model without having
   to cast it.
*/
BookmarkModel *BookmarkList::getBookmarkModel() const
{
    return bookmarkModel;
}

/*! \fn bool BookmarkList::addItem(const Bookmark &bookmark,bool refresh=false)
    Adds a new Bookmark to the list's data, and persists. If the Bookmark already exists
    within the list, it will not be added. If refresh is true, the view will
    also be updated, consequently it is possible to add multiple items to the
    list before updating it.

    Returns true if the Bookmark was added, false if it was not, e.g. because
    that Bookmark already exists in the list.

    \sa BookmarkModel::addItem()
*/
bool BookmarkList::addItem(const Bookmark &bookmark,bool refresh)
{
    // !!!! TODO - you're actually meant to call beginInsertRows(),
    // override insertRows(), then endInsertRows() on the model, but
    // the doco is not clear as to how this can be achieved using a
    // flat structure such as a list.

    // Get current selection (note that all this may not be necessary
    // if we can avoid using reset() in the BookmarkModel).
    int selectedIdx = -1;
    if ( !(selectionModel()->selectedIndexes().empty()) ) {
        selectedIdx = selectionModel()->selectedIndexes().first().row();
    }

    // Add the Bookmark - if it already exists in the list, rc will be false.
    bool rc = bookmarkModel->addItem(bookmark,refresh);
    if ( rc ) {
        // Persist all the current bookmarks.
        bookmarkModel->write();

        if ( refresh ) {
            // IF we need to refresh, then we also need to persist.
            bookmarkModel->write();
        }


        if ( selectedIdx > -1 ) {
            // Reselect. Item is appended, so this is safe.
            QModelIndex modelIndex = bookmarkModel->createIdx(selectedIdx);
            selectionModel()->select(modelIndex,QItemSelectionModel::Select);
        }
    }

    return rc;
}

/*! \fn Bookmark BookmarkList::getItem(int row) const
    Retrieves and returns the item at the given row index. Assumes that row is valid.
*/
Bookmark BookmarkList::getItem(int row) const
{
    return bookmarkModel->getItem(row);
}

/*! \fn void BookmarkList::select(int row)
    Selects the item at the given row. Assumes that row is valid.
*/
void BookmarkList::select(int row)
{
    QModelIndex modelIndex = bookmarkModel->createIdx(row);
    select(modelIndex);
}

// Selects the item at the given row. Assumes that row is valid.
void BookmarkList::select(const QModelIndex &idx) {

    if ( selectionModel()->selectedIndexes().empty() ) {
        selectionModel()->select(idx,QItemSelectionModel::Select);
    } else {
        selectionModel()->select(idx,QItemSelectionModel::ClearAndSelect);
    }
    selectionModel()->setCurrentIndex(idx,QItemSelectionModel::Current);
}

/*! \fn void BookmarkList::setDisplayMode(bool displaymode)
   Changes the display mode. If displayMode is true, the user will be able
   to navigate the list (i.e. Navigation mode). If displayMode is false, the list changes
   to Reorganise mode.
   Emits the signal displayModeChanged(bool).
*/
void BookmarkList::setDisplayMode(bool _displayMode)
{
    displayMode = _displayMode;

    delegate->setDisplayMode(displayMode);

    // Update the view.
    viewport()->update();

    emit displayModeChanged(displayMode);
}

// Called when the Select (OK) button is activated. Finds the currently
// selected Bookmark (or the first in the selection, if any) and emits
// the signal bookmarkSelected(Bookmark).
void BookmarkList::selectCurrent()
{
    // Get currently selected bookmark, if any.
    if ( !(selectionModel()->selectedIndexes().empty()) ) {
        int row = selectionModel()->selectedIndexes().first().row();
        Bookmark bookmark = getItem(row);
        // Shoot a signal to tell the world about the current selection.
        emit bookmarkSelected(bookmark);
    }

    // Otherwise, nothing is currently selected, so do nothing.
}

/*! \fn void BookmarkList::keyPressEvent(QKeyEvent *event)
   Overrides the superclass functionality to handle the following keys:
   Select/Ok: Calls selectCurrent(), which emits the singal bookmarkSelected(Bookmark).
   Up: When in Reorganise mode, moves the currently selected item up.
   Down: When in Reorganise mode, moves the currently selected item down.
   Back: When in Reorganise mode, returns to Navigation mode.
   All other cases revert to the default functionality.
*/
void BookmarkList::keyPressEvent(QKeyEvent *event)
{
    if ( event->isAutoRepeat() ) {
        return;
    }

    if ( displayMode ) {
        // Navigation mode.
        if ( event->key() == Qt::Key_Select ) {
            // Ok key has been pressed. Emit signal that a Bookmark has been selected.
            selectCurrent();
        } else {
            // Some other key -- handle as per usual.
            QListView::keyPressEvent(event);
        }
        return;
    }

    // Reorganise mode.
    if ( (event->key() == Qt::Key_Up) || (event->key() == Qt::Key_Down) ) {
        if ( selectionModel()->selectedIndexes().empty() ) {
            // Nothing selected, so we can't move anything anywhere.
            QListView::keyPressEvent(event);
            return;
        }

        // Get current selection (oldRow), and calculate newRow (i.e. where we're moving to).
        int oldRow = selectionModel()->selectedIndexes().first().row();
        int newRow;
        if ( event->key() == Qt::Key_Up ) {
            newRow = oldRow - 1;
        } else {
            newRow = (oldRow + 1);
        }

        // Wrap around if necessary.
        int idx1, idx2; // idx1 shall be removed and re-inserted at idx2
        if ( newRow < 0 ) {
            idx1 = 0;
            idx2 = bookmarkModel->rowCount()-1;
        } else if ( newRow >= bookmarkModel->rowCount() ) {
            idx1 = bookmarkModel->rowCount()-1;
            idx2 = 0;
        } else {
            idx1 = qMax(newRow,oldRow);
            idx2 = qMin(newRow,oldRow);
        }
        // Swap.
        Bookmark tmp = bookmarkModel->getItem(idx1);
        bookmarkModel->remove(idx1,false);
        bookmarkModel->insert(idx2,tmp,true);

        // Now that we've done the swap, write the revised version of the data.
        bookmarkModel->write();

        // Fix up the selection.
        if ( (newRow < 0) || (newRow >= bookmarkModel->rowCount()) ) {
            newRow = idx2;
        }
        QModelIndex idxNew = bookmarkModel->createIdx(newRow);
        select(idxNew);
    } else if ( event->key() == Qt::Key_Select ) {
        // Ok key has been pressed. Emit signal that a Bookmark has been selected.
        selectCurrent();
    } else if ( event->key() == Qt::Key_Back ) {
        // Return to Navigation mode.
        setDisplayMode(true);
    } else {
        // Some other key.
        QListView::keyPressEvent(event);
    }
}

