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

#include "bookmarksui.h"

#include "bookmark.h"
#include "bookmarklist.h"
#include "bookmarkmodel.h"
#include "helppreprocessor.h"

#include <QLabel>
#include <QModelIndex>
#include <QAbstractItemView>
#include <QUrl>
#include <QString>
#include <QList>
#include <QVBoxLayout>
#include <QToolBar>
#include <QMenuBar>
#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#endif
#include <QItemSelection>


/*!
  \internal
  \class BookmarksUI
  \brief A simple UI for a list of Bookmark objects.

  \mainclass
  This dialog displays the user's set of Bookmark objects, and also allows the user
  to delete Bookmarks. If the user chooses a Bookmark for display, the dialog emits
  the \l{function}{BookmarksUI::bookmarkSelected(Bookmark)} signal and closes
  (i.e. this UI is not responsible for the display of the actual documents).

  Bookmarks are not sorted -- the user may choose the ordering of the Bookmarks.
  This is achieved by toggling between Navigation mode and Reorganise
  mode. The modes are handled by the BookmarkList class.

  \sa HelpBrowser
  \sa BookmarkList
  \sa Bookmark
*/


/*! \fn BookmarksUI::BookmarksUI(QWidget *parent = 0)
  Initialiszs the UI by loading any existing Bookmark data from secondary storage.
*/
BookmarksUI::BookmarksUI(QWidget *parent) : QDialog(parent), listUI(0)
{
    init();
}

/*! \fn BookmarksUI::~BookmarksUI()
  When the UI is destroyed, the current set of bookmarks is written out to secondary
  storage.
*/
BookmarksUI::~BookmarksUI()
{
    // Write out data. NOTE that this has to be done here, because it is possible for the user
    // to add bookmarks without ever displaying the UI, and also because the close event is not
    // triggered when the user leaves the UI via the toolbar.
    //write();
}

// Called by the ctor.
void BookmarksUI::init()
{
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    // Put in a title.
    QLabel *label = new QLabel(tr("Bookmarks"));
    label->setAlignment(Qt::AlignHCenter);
    layout->addWidget(label);

    // Create the list - this also loads the data.
    listUI = new BookmarkList(this);
    //listUI->setSortingEnabled(true);
    // Make sure the first item (if any) is selected.
    if ( listUI->getBookmarkModel()->rowCount() > 0 ) {
        listUI->select(0);
    }

    // Set selection mode for extended selection, allowing for multiple selection using
    // the Ctrl or Shift keys.
    listUI->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // The Delete menu item gets enabled/disabled, depending on whether or not anything is selected.
    connect(listUI->selectionModel(),SIGNAL(selectionChanged(const QItemSelection &,const QItemSelection &)),
            this,SLOT(selectionChanged()));
    // When the Select button is activated, this object is notified and the Url for the
    // current Bookmark is displayed.
    connect(listUI,SIGNAL(bookmarkSelected(Bookmark)),this,SLOT(handleBookmarkSelected(Bookmark)));
    // When listUI's display mode changes from Navigate to Reorganise or vv, this object
    // must be notified so that the menu action (reorganiseAction) can have its label text
    // toggled.
    connect(listUI,SIGNAL(displayModeChanged(bool)),this,SLOT(handleDisplayModeChanged(bool)));

    layout->addWidget(listUI);

#ifdef QTOPIA4_TODO
    setToolBarsMovable( false );
#endif

    // Set up the context menu, with its Delete option.
    deleteAction = new QAction(QIcon(":icon/trash"), tr("Delete Bookmark(s)"), this);
    deleteAction->setWhatsThis( tr( "Delete currently selected bookmark(s)." ) );
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteBookmarks()));

    // Set up the Reorganise action. Note that this menu action will have its text
    // toggled between reorganiseMenuText and navigationMenuText. When it is activated,
    // the mode will change.
    reorganiseAction = new QAction(QIcon(":icon/shuffle"), tr("Reorganise"), this);
    reorganiseAction->setWhatsThis(tr("Change the ordering of the bookmarks"));
    reorganiseAction->setCheckable(true);
    connect(reorganiseAction,SIGNAL(triggered(bool)),this,SLOT(setReorganise(bool)));

#ifdef QTOPIA_PHONE
    // Recreate context menu without the Help option.
    contextMenu = QSoftMenuBar::menuFor(this);
    QSoftMenuBar::setHelpEnabled( this, false );
    contextMenu->addAction(deleteAction);
    contextMenu->addAction(reorganiseAction);
#else
    QToolBar* toolbar = new QToolBar(this);
    addToolBar(toolbar);
#ifdef QTOPIA4_TODO
    toolbar->setHorizontalStretchable( true );
#endif
    QMenuBar *menu = new QMenuBar( toolbar );
    toolbar->addWidget(menu);
    toolbar = new QToolBar( this );
    addToolBar(toolbar);
    toolbar->addAction(deleteAction);
    toolbar->addAction(reorganiseAction);
#endif

    // Load in the data, if any.
    //load();

    // Make sure we start off with the right status on the widgets.
    selectionChanged();
}

/*! \fn void BookmarksUI::bookmarkSelected(Bookmark)
  Signal to indicate that the user has chosen a specific Bookmark for display.
  Typically, this will be handled by displaying the contents of the URL.
*/

// Private slot. When the user chooses the Delete action, the currently selected Bookmark(s) will
// be removed. Note that the classes handle multiple selection so that they can be used
// outside Qtopia.
void BookmarksUI::deleteBookmarks()
{
    // Get currently selected row(s), if any.
    QModelIndexList selection = listUI->selectionModel()->selectedIndexes();
    // Need to sort the list so we can delete backwards.
    qSort(selection);

    // Iterate backwards through the list of selected rows, removing those rows from the
    // list.
    BookmarkModel *model = listUI->getBookmarkModel();
    for ( QList<QModelIndex>::iterator i = --(selection.end()) ; i >= selection.begin(); --i ) {
        model->remove(i->row());
    }
    // Persist the revised set of bookmarks.
    model->write();

    // If any were selected, get the first in the selection and reselect that index if possible.
    if ( !selection.empty() ) {
        listUI->select(selection.first().row());
    }
}

// Private slot. When the selection changes, the Delete item in the menu may be enabled/disabled.
void BookmarksUI::selectionChanged()
{
    // The Delete action in the menu should only be enabled if there's something selected
    // (i.e. something to delete).
    deleteAction->setEnabled(!(listUI->selectionModel()->selectedIndexes().empty()));
}

/*! \fn bool BookmarksUI::addBookmark(const QUrl &url,const QString &title)
  Adds a new Bookmark. Returns false if the Bookmark already existed in the collection.
*/
bool BookmarksUI::addBookmark(const QUrl &url,const QString &title)
{
    // Add a new Bookamrk with the given parameters. Returns false if the list already
    // contained a Bookmark like this (i.e. it was not added again).
    bool rc = listUI->addItem(Bookmark(url,title),true);

    // If we've just added the first item, make sure it's selected.
    if ( rc && (listUI->getBookmarkModel()->rowCount() == 1) ) {
        listUI->select(0);
    }

    return rc;
}

// Private slot which handles the BookmarkList::bookmarkSelected(Bookmark) signal. Emits its
// own bookmarkSelected(Bookmark) signal, and closes the UI, i.e. it is up to
// the parent to display the Bookmark's Url.
void BookmarksUI::handleBookmarkSelected(Bookmark bookmark)
{
    // Tell the world that the Select button (OK bn) has been applied to a Bookmark.
    emit bookmarkSelected(bookmark);
    // Close the Bookmarks UI -- it is up to handlers of the signal to display the URL.
    close();
}

// Private slot which handles the BookmarkList::displayModeChanged(bool) signal.
// Modifies the text of reorganiseAction so that it reflects the current mode of
// the list, and ensures reorganiseAction's status is also correct.
void BookmarksUI::handleDisplayModeChanged(bool displayMode)
{
    // When the list's display mode changes, we must update the label on the menu item.
    if ( displayMode ) {
        // The menu action should now allow them to reorganise.
        reorganiseAction->setText(tr("Reorganise"));
        reorganiseAction->setWhatsThis(tr("Change the ordering of the bookmarks"));
    } else {
        // The menu action should now allow them to get back to navigation.
        reorganiseAction->setText(tr("Navigate"));
        reorganiseAction->setWhatsThis(tr("Return to navigation mode."));
    }

    // Ensure that the reorganiseAction is correctly set.
    if ( reorganiseAction->isChecked() == displayMode ) {
        reorganiseAction->setChecked(!displayMode);
    }
}

/*! \fn void BookmarksUI::setReorganise(bool checked)
    Handles reorganiseAction's triggered(bool) signal. Modifies the BookmarkList's
    mode in response to the given status. This will cause
    BookmarkList::displayModeChanged(bool) to be emitted.
*/
void BookmarksUI::setReorganise(bool checked)
{
    // Sets the list's mode to Navigation mode if reorganise is off,
    // or Reorganise mode if reorganise is on.
    listUI->setDisplayMode(!checked);

    // This triggers displayModeChanged(bool), which is handled in this class
    // by handleDisplayModeChanged(bool).
}

