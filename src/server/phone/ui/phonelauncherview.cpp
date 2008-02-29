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

#include "phonelauncherview.h"

#include "selecteditem.h"
#include "griditem.h"
#include "animator_p.h"
#include "animatorfactory_p.h"

#include <QApplication>
#include <QPixmap>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QFocusEvent>
#include <QColor>
#include <qtopiaapplication.h>


const QString PhoneLauncherView::SELECTED_BACKGROUND_FILE_MOUSE_PREFERRED(":image/phone/launcher_icon_sel_light");
const QString PhoneLauncherView::SELECTED_BACKGROUND_FILE_DEFAULT(":image/phone/launcher_icon_sel");


/*!
  \class PhoneLauncherView

  \brief The PhoneLauncherView class is a view onto a scene, which is a grid of objects.

  PhoneLauncherView creates and maintains the scene itself; it is capable of creating and
  adding the grid objects which will populate the scene's grid. Each grid object represents
  a folder or application, i.e. something that can be 'opened'.

  In addition to the grid objects, a 'selected' item is positioned over the 'current'
  grid object. The selected item has a different appearance to its current grid object, and is
  also able to animate.

  See also PhoneBrowser.
*/

// Grid objects are modelled by the GridItem class. The selected item is modelled by the
// SelectedItem class.
//
// See also GridItem and SelectedItem
//
// Events:
//
// PhoneLauncherView listens for mouse events and keyboard events. Clicking on a GridItem
// object (mouse press followed by mouse release) causes that GridItem to become current (i.e.
// SelectedItem will now be positioned over it) and activated (i.e. the GridItem's underlying
// application will be invoked). Pressing any of the keyboard characters listed in 'iconMapping' also
// causes the GridItem at that index to become current and activated.
//
// Note that SelectedItem also listens for keyboard events -- it handles the arrow keys, that enable
// it to shift horizontally or vertically to a neighbour. Consequently, SelectedItem must always have
//keyboard focus.


/*!
  \fn PhoneLauncherView::PhoneLauncherView(int rows, int cols, const QString &mapping,const QString &animator,const QString &animatorBackground,QWidget *parent=0)

  \a rows: Number of rows that will be in the grid.
  \a cols: Number of columns that will be in the grid.
  \a mapping: A list of characters that map to grid objects according to their position. For example,
  the first character in mapping will refer to the grid object at row 0, column 0, the second
  character will refer to row 0, column 1 etc. The mapping is used by \l{function}{keyPressEvent(QKeyEvent *)}.
  \a animator: The name of an animation object, which generally comes from a configuration file. Note that this name must be known to AnimatorFactory, which is responsible for the creation of the animators. The string may be an empty string, in which case there will be no animation. Note, however, that if any movie files (.mng) are present for a QContent object, these will take precedence over any animator.
  \a animatorBackground: The name of an animation object (to be used for the background during animation), which generally comes from a configuration file. Note that this name must be known to AnimatorFactory, which is responsible for the creation of the animators. The string may be an empty string, in which case there will be no background animation.
  \a parent: Optional parent widget.
*/
PhoneLauncherView::PhoneLauncherView(int _rows, int _columns, const QString &mapping,
                                     const QString &_animatorDescription,const QString &_animatorBackgroundDescription,
                                     QWidget *parent)
    : QGraphicsView(parent)
    , rows(_rows)
    , columns(_columns)
    , scene(0)
    , selectedItem(0)
    , pressedItem(0)
    , animatorDescription(_animatorDescription)
    , animatorBackground(0)
    , iconMapping(mapping)
    , margin(MARGIN_DEFAULT)
{
    // Avoid using Base brush because it is semi-transparent (i.e. slow).
    setBackgroundRole(QPalette::Window);

    // We know that the Window brush is completely transparent,
    // so we can save some cycles by not painting it.
    viewport()->setAutoFillBackground(false);

    // Create & populate the scene.
    scene = new QGraphicsScene();
    setScene(scene);

    // Calculate the margin for the SelectedItem.
    bool isMousePreferred = Qtopia::mousePreferred();
    if ( isMousePreferred ) {
        margin = MARGIN_MOUSE_PREFERRED;
    } else {
        margin = MARGIN_DEFAULT;
    }

    // !!!!!!!!!!!!!HACK FOR QT BUG
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Calculate the selectedItem's background pixmap file.
    QString backgroundFileName;
    if ( isMousePreferred ) {
        backgroundFileName = SELECTED_BACKGROUND_FILE_MOUSE_PREFERRED;
    } else {
        backgroundFileName = SELECTED_BACKGROUND_FILE_DEFAULT;
    }

    // Create and add selectedItem.
    addSelectedItem(backgroundFileName,DEFAULT_MOVE_TIME_DURATION);

    // Create the shared animator background object.
    animatorBackground = AnimatorFactory::getAnimator(_animatorBackgroundDescription);
}

/*!
  \fn PhoneLauncherView::~PhoneLauncherView()
*/
PhoneLauncherView::~PhoneLauncherView()
{
     delete scene;
     if ( animatorBackground )
         delete animatorBackground;
}

/*!
  \fn void PhoneLauncherView::clicked(QContent content)
  Indicates that an item on the grid with the given content has been selected. Typically, this
  is handled by invoking that item's underlying application.
  \a content See description.
*/

/*!
  \fn void PhoneLauncherView::highlighted(QContent content)
  Indicates that the item on the grid with the given content has become the new current item.
  \a content See description.
*/

/*!
  \internal
  \fn GridItem *PhoneLauncherView::createItem(QContent *content,int row,int column) const
  Creates and returns a new GridItem object, adding it to the SelectedItem and to the scene.
  \a content: Contains information to be used by the GridItem object, such as the
  Icon used to display it, the name of its underlying application, etc.
  \a row: Item's row position in the grid (starts from 0).
  \a column: Item's column position in the grid (starts from 0).
*/
GridItem *PhoneLauncherView::createItem(QContent *content,int row,int column) const
{
    Animator *animator = 0;
    Animator *background = 0;
    bool loadMovie = false;

    if ( content ) {
        // The Animator objects could be shared by all GridItems -
        // however, we have not done that here, since the Animators may need to cache
        // values (specific to GridItem objects) to speed up processing - so for now, each
        // GridItem has its own Animator, although that design decision may be reversed in
        // future.
        // Note: the animatorBackground IS shared among GridItems and thus must be cleaned up
        // by the phonelauncherview.
        // Note: there is no memory leak here, since QGraphicsScene will delete all of its
        // contained graphical items upon destruction. Note also that the new object has no
        // size - its size will be calculated and assigned during resizeEvent(...).
        animator = AnimatorFactory::getAnimator(animatorDescription);
        background = animatorBackground;

        // If an animator has been specified (regardless of whether or not it was successfully
        // created), the GridItem SHOULD NOT attempt to load any movie files, so leave 'loadMovie'
        // as false.
        if ( (animatorDescription.isNull()) || (animatorDescription.isEmpty()) ) {
            loadMovie = true;
        }
    }

    GridItem *item = new GridItem(content,row,column,rows,columns,
                            animator,background,loadMovie);

    // Add the item to both the scene and the SelectedItem.
    if ( selectedItem->addItem(item) ) {
        scene->addItem(item);
    } else {
        qWarning("PhoneLauncherView::createItem(...): Failed to add new GridItem for row %d, column %d",row,column);
    }

    return item;
}

/*!
  \internal
  \fn void PhoneLauncherView::addSelectedItem(const QString &backgroundFileName,int moveTimeDuration)
  Called by constructor to create and add the SelectedItem, which highlights the current
  GridItem object.
  \a backgroundFileName: an image file used to supply a background for the SelectedItem's image.
  \a moveTimeDuration: time, in milliseconds, that it should take to move the SelectedItem
  from one GridItem to the next in response to a keyboard command.
*/
void PhoneLauncherView::addSelectedItem(const QString &backgroundFileName,int moveTimeDuration)
{
    if ( !scene ) {
        qWarning("PhoneLauncherView::addSelectedItem(...): Can't populate scene -- need scene.");
        return;
    }

    // Make the moveable SelectedItem object that slides is positioned over the current
    // GridItem object. Note: there is no memory leak here, since QGraphicsScene will delete
    // all of its contained graphical items.
    selectedItem = new SelectedItem(backgroundFileName,margin,moveTimeDuration);
    // Signal for when a GridItem object is invoked, eg by pressing the Qt::Select key.
    connect(selectedItem->getConnector(),SIGNAL(itemSelected(GridItem *)),this,SLOT(itemSelectedHandler(GridItem *)));
    // Signal for when the current GridItem object changes.
    connect(selectedItem->getConnector(),SIGNAL(selectionChanged(GridItem *)),this,SLOT(selectionChangedHandler(GridItem *)));

    // Make sure that selectedItem is higher in the stacking order than the (default) GridItem objects.
    selectedItem->setZValue(selectedItem->zValue()+1);

    scene->addItem(selectedItem);

    // The next 2 statements ensure that the SelectedItem can get keyboard events.
    selectedItem->setSelected(true);
    scene->setFocusItem(selectedItem);

    // The other items (GridItem objects) are added by external calls of addItem(...)
}

/*!
  \internal
  \fn void PhoneLauncherView::getItemDimensions(int &itemWidth,int &itemHeight) const
  Retrieves the appropriate width and height for each GridItem object, according to the
  current dimensions of the view.
*/
void PhoneLauncherView::getItemDimensions(int &itemWidth,int &itemHeight) const
{
    // Get the pixel size of the view's margins.
    int left,top,right,bottom;
    getContentsMargins(&left,&top,&right,&bottom);

    int viewWidth = size().width() - left - right - 1;
    int viewHeight = size().height() - top - bottom - 1;
    itemWidth = viewWidth/columns;
    itemHeight = viewHeight/rows;
}

/*!
  \internal
  \fn void PhoneLauncherView::getRowAndColumn(int idx,int &row,int &column) const
  Translates an item index (one-dimensional) into a row and a column (two-dimensional)
  as used by the SelectedItem to refer to the GridItem objects.
*/
void PhoneLauncherView::getRowAndColumn(int idx,int &row,int &column) const
{
    row = idx/columns;
    column = idx - (columns*row);
}

/*!
  \fn void PhoneLauncherView::addItem(QContent* content, int pos)
  Creates a new GridItem object and adds it to the scene, or else updates the GridItem
  at the given position, if it already exists. When a new GridItem object is added,
  its position and size will be calculated and set in response to resizeevent(...).
  When an existing GridItem object is modified, its position and size do not
  change in response to \a content -- position and size are determined by other factors,
  such as the size of the view and the current resolution.

  \a content: Contains information to be used by the GridItem object, such as the
  Icon used to display it, the name of its underlying application, etc.
  \a pos: The index of the GridItem object, which will be converted into a
  row and a column position.
*/
void PhoneLauncherView::addItem(QContent* content, int pos)
{
    if ( !selectedItem ) {
        qWarning("PhoneLauncherView::addItem(...): Could not add GridItem for pos %d - no selected item.",pos);
        return;
    }

    // Calculate row and column from pos.
    int row, column;
    getRowAndColumn(pos,row,column);

    GridItem *item = 0;
    if ( (item = selectedItem->getItem(row,column)) ) {
        // Already have an item for that row and column. That means we need to update it, not
        // create it.
        item->setContent(content);
        scene->update();
    } else {
        // Create the new GridItem object.
        createItem(content,row,column);
    }
}

/*!
  \fn QContent *PhoneLauncherView::currentItem() const
  Returns the content information for the current GridItem object, or 0 if no item is current.
*/
QContent *PhoneLauncherView::currentItem() const
{
    GridItem *currentItem = getCurrentItem();
    if ( currentItem ) {
        return currentItem->getContent();
    } else {
        return 0;
    }
}

/*!
  \internal
  \fn GridItem *PhoneLauncherView::getCurrentItem() const
  Returns the current GridItem object, or 0 if no item is current.
*/
GridItem *PhoneLauncherView::getCurrentItem() const
{
    if ( selectedItem ) {
        return selectedItem->getCurrent();
    } else {
        return 0;
    }
}

/*!
  \fn void PhoneLauncherView::setCurrentItem(int idx)
  Causes the item positioned at the given index to become the current item. If \a idx is
  invalid, no change will occur.
  \a idx: The index of a GridItem object.
*/
void PhoneLauncherView::setCurrentItem(int idx)
{
    if ( selectedItem ) {
        // Calculate the row and column of the required item according to the index.
        int row, column;
        getRowAndColumn(idx,row,column);

        // Make sure we've actually got an item for that row and column, even if it's
        // an item with empty content.
        GridItem *item = selectedItem->getItem(row,column);
        if ( !item ) {
            item = createItem(0,row,column);
        }

        selectedItem->setCurrent(item,false); // 2nd param means no animation
    }
}

/*!
  \fn void PhoneLauncherView::setBusy(bool flag)
  Affects the appearance of the SelectedItem object.
  \a flag: If true, the SelectedItem will take on its 'active' appearance.
*/
void PhoneLauncherView::setBusy(bool flag)
{
    if ( selectedItem ) {
        selectedItem->setActive(flag);
    }
}

/*!
  \fn void PhoneLauncherView::updateImages()
  Causes all GridItem objects to query their QContent counterparts to refresh their image
  information. This function should be called when image data has/may have changed.
*/
void PhoneLauncherView::updateImages()
{
    if ( selectedItem ) {
        selectedItem->updateImages();
    }
}

/*!
  \internal
  \fn void PhoneLauncherView::resizeEvent(QResizeEvent *event)
  Resizes all the scene objects, relative to the new size of this view.
*/
void PhoneLauncherView::resizeEvent(QResizeEvent *event)
{
    scene->setSceneRect(0,0,event->size().width(),event->size().height());    //MWB

    if ( !selectedItem ) {
        return;
    }

    // Get the width and height of each item.
    int itemWidth;
    int itemHeight;
    getItemDimensions(itemWidth,itemHeight);

    // Loop through all the GridItem objects and reset their sizes.
    for ( int row = 0; row < rows; row++ ) {
        for ( int col = 0; col < columns; col++ ) {
            GridItem *item = selectedItem->getItem(row,col);
            if ( !item ) {
                // Create and add an empty content for this row and column.
                item = createItem(0,row,col);
            }

            // Calculate the item's position in the scene.
            int x = col * itemWidth;
            int y = row * itemHeight;
            item->setRect(x,y,itemWidth,itemHeight);
        }
    }

    // Ask the SelectedItem to also recalculate its size.
    selectedItem->resize();

    QGraphicsView::resizeEvent(event);
}

/*!
  \internal
  \fn void PhoneLauncherView::focusInEvent(QFocusEvent *event)
  Ensures that when the PhoneLauncherView is focused after an activity such as launching an
  application, the current icon will animate.
*/
void PhoneLauncherView::focusInEvent(QFocusEvent *event)
{
    QGraphicsView::focusInEvent(event);

    if ( selectedItem && event->reason() != Qt::PopupFocusReason) {
        selectedItem->startAnimating();
    }
}

/*!
  \internal
  \fn void PhoneLauncherView::itemSelectedHandler(GridItem *item)
  Called in response to the current item being activated (e.g. by Ok button, by mouse click, etc).
  Extracts information from the item and emits the signal clicked(QContent).
  Signals emitted: \l{function}{clicked(QContent)}.
*/
void PhoneLauncherView::itemSelectedHandler(GridItem *item)
{
    if ( !item ) {
        qWarning("PhoneLauncherView::itemSelectedHandler(...): Error - no item in itemSelectedHandler");
        return;
    }

    if ( item->getContent() && !(item->getContent()->name().isEmpty()) ) {
        // Activate the item - this changes its appearance.
        if ( selectedItem ) {
            selectedItem->setActive(true);
        }
        emit clicked(*(item->getContent()));
    }
}

/*!
  \internal
  \fn void PhoneLauncherView::selectionChangedHandler(GridItem *item)
  Called in response to the current item changing (i.e. SelectedItem has shifted position,
  highlight a different GridItem object). Updates the window title, and emits the
  signal highlighted(...).
  Signals emitted: \l{function}{highlighted(QContent)}.
*/
void PhoneLauncherView::selectionChangedHandler(GridItem *item)
{
    if ( !item ) {
        qWarning("PhoneLauncherView::selectionChangedHandler(...): Error - item parameter is null.");
        // Set the title to a space rather than an empty string. This causes the title bar to
        // remain the same, but just without a title. An empty string will cause a different kind of
        // title bar to appear.
        topLevelWidget()->setWindowTitle(" ");
        return;
    }

    if ( !(item->getContent()) ) {
        // Set the title to a space rather than an empty string. This causes the title bar to
        // remain the same, but just without a title. An empty string will cause a different kind of
        // title bar to appear.
        topLevelWidget()->setWindowTitle(" ");
        return;
    }

    emit highlighted(*(item->getContent()));
    topLevelWidget()->setWindowTitle(item->getContent()->name());
}

/*!
  \internal
  \fn GridItem *PhoneLauncherView::getItemAt(const QPoint &point) const
  Returns the GridItem object that resides at the given pixel position.
*/
GridItem *PhoneLauncherView::getItemAt(const QPoint &point) const
{
    if ( !selectedItem ) {
        return 0;
    }

    // Get the item at position 'point'.
    int itemWidth, itemHeight;
    getItemDimensions(itemWidth,itemHeight);
    int column = point.x()/itemWidth;
    int row = point.y()/itemHeight;

    return selectedItem->getItem(row,column);
}

/*!
  \internal
  \fn void PhoneLauncherView::keyPressEvent(QKeyEvent *event)
  If a key that is listed in the icon mapping is pressed, the SelectedItem's appearance
  changes to be not 'active', and the GridItem object which maps to that key becomes
  the current item.
  Signals emitted: \l{function}{highlighted(QContent)}, \l{function}{clicked(QContent)}.
*/
void PhoneLauncherView::keyPressEvent(QKeyEvent *event)
{
    const char a = event->text()[0].toLatin1();
    int index = iconMapping.toLatin1().indexOf(a);

    if ( selectedItem && (index >= 0) && (index < rows * columns) ) {
        // Change the selected item's appearance.
        selectedItem->setActive(false);

        // Find out what row/column index maps to, and set that GridItem to
        // be current (i.e. SelectedItem will shift over to it).
        int row, column;
        getRowAndColumn(index,row,column);
        // setCurrent(int,int) causes selectionChanged() to be emitted, which invokes
        // this object's selectionChangedHandler(...) slot, which updates the window
        // title and emits the signal highlighted(...).
        selectedItem->setCurrent(row,column);

        // We also cause the item to be activated, i.e. its underlying application will be
        // invoked. Note that this auses SelectedItem's connector's itemSelected(...)
        // signal to be emitted, which is connected to this object's slot itemSelectedHandler(),
        // which emits clicked(...).
        selectedItem->triggerItemSelected(selectedItem->getCurrent());
    } else {
        // Key not handled by PhoneLauncherView.
        QGraphicsView::keyPressEvent(event);
    }
}

/*!
  \internal
  \fn void PhoneLauncherView::mousePressEvent(QMouseEvent *event)
  Stores the GridItem object that is being clicked.
*/
void PhoneLauncherView::mousePressEvent(QMouseEvent *event)
{
    pressedItem = getItemAt(event->pos());
}

/*!
  \internal
  \fn void PhoneLauncherView::mouseReleaseEvent(QMouseEvent *event)
  If a GridItem object has been clicked, it becomes the current item (i.e. SelectedItem
  will highlight it) and signals will be issued to indicate that that particular item
  has been invoked.
  Signals emitted: \l{function}{highlighted(QContent)}, \l{function}{clicked(QContent)}.
*/
void PhoneLauncherView::mouseReleaseEvent(QMouseEvent *event)
{
    GridItem *releasedItem = getItemAt(event->pos());

    if ( releasedItem && (pressedItem == releasedItem) ) {
        // We've got an item that we've released the button on, and it's the
        // same as the one we've pressed on.
        if ( selectedItem ) {
            if ( selectedItem->getCurrent() != pressedItem ) {
                // This is different to the current item - so we make the new one current.
                // Note that this causes SelectedItem's connector's selectionChanged(...)
                // signal to be emitted, which is connected to this object's slot
                // selectionChangedHandler(), which emits highlighted(...) and causes the
                // window title to be updated.
                selectedItem->setCurrent(pressedItem);
            }

            // The clicked-on item becomes activated, i.e. its underlying application will be
            // invoked. Note that this auses SelectedItem's connector's itemSelected(...)
            // signal to be emitted, which is connected to this object's slot itemSelectedHandler(),
            // which emits clicked(...).
            selectedItem->triggerItemSelected(selectedItem->getCurrent());
        }
    }

    pressedItem = 0;

    // If a double-click occurs the SelectedItem loses its selection for some reason, so it is
    // advisable to do the following.
    if ( selectedItem ) {
        selectedItem->setSelected(true);
    }
 }
