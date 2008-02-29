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

#include "selecteditem.h"
#include "griditem.h"
#include "selecteditemconnector.h"
#include "animator_p.h"

#include <QGraphicsScene>
#include <QPixmap>
#include <QMovie>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QApplication>
#include <QKeyEvent>
#include <QTimeLine>
#include <QSvgRenderer>
#include <QTimer>


/*!
  \internal
  \class SelectedItem

  \brief   SelectedItem is a graphical object which highlights the currently selected GridItem
  object in the PhoneLauncherView's grid.

  \mainclass
  The SelectedItem maintains pointers to all of the GridItem objects in the grid, stored in
  a GridItemTable. This enables the SelectedItem to accept certain keyboard events which
  enable it to move across from the current item to its neighbours in the indicated Direction
  (Up, Down, Left or Right). The SelectedItem is also able to animate whenever a new GridItem
  object becomes current.

  The SelectedItem relies on the current GridItem object for both sizing information and the
  QPixmap, QMovie and/or QSvgRenderer that it uses to display itself.

  Since SelectedItem does not inherit from QObject, it contains a SelectedItemConnector to
  handle callbacks, thus avoiding multiple inheritance.

  \sa SelectedItemConnector
  \sa GridItem
*/


/*!
  \internal
  \fn SelectedItem::SelectedItem(const QString &backgroundFileName,int margin,int slideTimeInterval=500,QGraphicsScene *scene=0)
  \a backgroundFileName: Name of the file used to display the image's background.
  \a margin: The SelectedItem is larger than the GridItem that it magnifies. The 'margin' attribute
   is the number of extra pixels added to each border of the current GridItem, affecting
   the SelectedItem's size and position. For example, the width of SelectedItem is
   GridItem's width + (margin * 2).
   \a slideTimeInterval: Number of milliseconds that it should take for the SelectedItem to move
   across from the current GridItem object to a neighbour in the grid.
   \a scene: The (optional) scene on which this item should be drawn.
*/
SelectedItem::SelectedItem(const QString &_backgroundFileName,
                           int _margin,int _slideTimeInterval,QGraphicsScene *scene)
    : QGraphicsRectItem(0,scene)
    , highlightColor(qApp->palette().color(QPalette::Disabled,QPalette::Highlight))
    , backgroundHighlightColor(qApp->palette().color(QPalette::Disabled,QPalette::Button))
    , connector(0)
    , background(0)
    , backgroundFileName(_backgroundFileName)
    , margin(_margin)
    , selectedSize(-1,-1)
    , table()
    , currentItem(0)
    , destItem(0)
    , movie(0)
    , active(false)
    , moveTimeLine(0)
    , playTimeLine(0)
    , slideTimeInterval(_slideTimeInterval)
    , animationStage(0)
    , animationPending(false)
    , currentX(-1)
    , currentY(-1)
    , destX(-1)
    , destY(-1)
    , xDrift(-1)
    , yDrift(-1)
{
    // The item supports selection. Enabling this feature will enable setSelected()
    // to toggle selection for the item.
    setFlag(QGraphicsItem::ItemIsSelectable,true);

    // The item supports keyboard input focus (i.e., it is an input item). Enabling
    // this flag will allow the item to accept focus, which again allows the delivery
    // of key events to QGraphicsItem::keyPressEvent() and QGraphicsItem::keyReleaseEvent().
    setFlag(QGraphicsItem::ItemIsFocusable,true);

    // Create the connector object that supports the signals/slots mechanism.
    connector = new SelectedItemConnector(this);

    // Create the timeline that is responsible for moving the SelectedItem from one
    // GridItem to its neighbour.
    createMoveTimeLine();

    // And a timeline to control manual animation.
    createPlayTimeLine();
}

// Create the timeline that is responsible for moving the SelectedItem from one
// GridItem to its neighbour.
void SelectedItem::createMoveTimeLine()
{
    if ( moveTimeLine ) {
        delete moveTimeLine;
    }

    moveTimeLine = new QTimeLine(slideTimeInterval,connector);
    moveTimeLine->setFrameRange(0,100);
    QObject::connect(moveTimeLine,SIGNAL(frameChanged(int)),connector,SLOT(moving(int)));
    QObject::connect(moveTimeLine,SIGNAL(stateChanged(QTimeLine::State)),connector,SLOT(movingStateChanged(QTimeLine::State)));
}

// And a timeline to control manual animation.
void SelectedItem::createPlayTimeLine()
{
    if ( playTimeLine ) {
        delete playTimeLine;
    }

    playTimeLine = new QTimeLine(ANIMATION_TIME,connector);
    playTimeLine->setFrameRange(0,100);
    QObject::connect(playTimeLine,SIGNAL(frameChanged(int)),connector,SLOT(playing(int)));
    QObject::connect(playTimeLine,SIGNAL(stateChanged(QTimeLine::State)),connector,SLOT(animationStateChanged(QTimeLine::State)));
}

/*!
  \internal
  \fn SelectedItem::~SelectedItem()
*/
SelectedItem::~SelectedItem()
{
    // Make sure the movie isn't still running.
    detachAnimation();

    delete moveTimeLine;
    delete playTimeLine;
    delete connector;
    if ( background ) {
        delete background;
    }
}

/*!
  \internal
  \fn bool SelectedItem::addItem(GridItem *item)
  Adds a new item to the store, and returns true if the item was added successfully, or false if
  there was already an object in the item's row:column position.
*/
bool SelectedItem::addItem(GridItem *item)
{
    return table.add(item);
}

/*!
  \internal
  \fn GridItem *SelectedItem::getCurrent() const
  Returns the current GridItem object, or 0 if no item is current.
*/
GridItem *SelectedItem::getCurrent() const
{
    if ( destItem ) {
        return destItem;
    }

    return currentItem;
}

/*!
  \internal
  \fn GridItem *SelectedItem::getItem(int row,int column) const
  Returns the GridItem object that is stored at the given row and column position, or 0 if
  no object is stored at that position.
*/
GridItem *SelectedItem::getItem(int row,int column) const
{
    return table.getItem(row,column);
}

/*!
  \internal
  \fn QObject *SelectedItem::getConnector() const
  Returns the object responsible for handling the SelectedItem's signals and slots.
*/
QObject *SelectedItem::getConnector() const
{
    // Upcast.
    return static_cast<QObject *>(connector);
}

/*!
  \internal
  \fn QPoint SelectedItem::getPos(GridItem *_item) const
  Returns the appropriate pixel position for the SelectedItem when positioned over the given
  item.
*/
QPoint SelectedItem::getPos(GridItem *item) const
{
    int x = 0, y = 0;
    QSize _selectedSize = getSelectedSize(item);

    if ( item->getColumn() <= 0 ) { // sitting on left border
        x = qRound(item->rect().x());
    } else if ( item->getColumn() >= table.getTopColumn() ) { // sitting on right border
        x = qRound(item->rect().x() + item->rect().width() - _selectedSize.width());
    } else {
        // This object should be centred over the GridItem.
        x = qRound(item->rect().x() + (item->rect().width()/2) - _selectedSize.width()/2);
    }
    if ( item->getRow() <= 0 ) { // sitting on top row
        y = qRound(item->rect().y());
    } else if ( item->getRow() >= table.getTopRow() ) { // sitting on bottom row
        y = qRound(item->rect().y() + item->rect().height() - _selectedSize.height());
    } else {
        // This object should be centred over the GridItem.
        y = qRound(item->rect().y() + (item->rect().height()/2) - getSelectedSize(item).height()/2);
    }

    return QPoint(x,y);
}

/*!
  \internal
  \fn QSize SelectedItem::getSelectedSize(GridItem *item) const
  Returns the size for this SelectedItem, wrt the given GridItem object. The size
  of this item must be larger than the GridItem size, but must retain the same
  width/height ratio, and also be large enough to comfortably accommodate the
  selected image.
  Note that the dimensions are created on demand, and rely on the assumption that the
  GridItem's size and the selected image's size don't change at runtime.
*/
QSize SelectedItem::getSelectedSize(GridItem *item) const
{
    if ( selectedSize.width() == -1 || selectedSize.height() == -1 ) {
        // Recalculate the size.
        selectedSize.setWidth(qRound(item->rect().width()) + (margin * 2));
        selectedSize.setHeight(qRound(item->rect().height()) + (margin * 2));

        int minimum = item->getSelectedImageSize() + (margin * 2);

        if ( selectedSize.width() < minimum ) {
            // Increase width.
            int diff = minimum - selectedSize.width();
            selectedSize.setWidth(minimum);
            selectedSize.setHeight(selectedSize.height() + diff);
        }
        if ( selectedSize.height() < minimum ) {
            // Increase height.
            int diff = minimum - selectedSize.height();
            selectedSize.setHeight(minimum);
            selectedSize.setWidth(selectedSize.width() + diff);
        }
    }

    return selectedSize;
}

/*!
  \internal
  \fn QRect SelectedItem::getGeometry(GridItem *item) const
  Returns the appropriate pixel position and dimensions for the SelectedItem when positioned
  over the given item.
*/
QRect SelectedItem::getGeometry(GridItem *item) const
{
    // First, get the (x,y) top left-hand corner of this item, when positioned over
    // the GridItem.
    QPoint point = getPos(item);

    // Also width & height.
    QSize selectedSize = getSelectedSize(item);
    int w = selectedSize.width();
    int h = selectedSize.height();

    return QRect(point.x(),point.y(),w,h);
}

/*!
  \internal
  \fn void SelectedItem::drawBackground(QPainter *painter)
  The background image is created on demand, within this method.
*/
void SelectedItem::drawBackground(QPainter *painter)
{
    QRect rectangle = rect().toRect();

    if ( !background ) {
        // Create on demand.
        QImage bgImage(backgroundFileName);

        blendColor(bgImage,backgroundHighlightColor);
        background = new QPixmap(QPixmap::fromImage(bgImage.scaled(rectangle.width(),rectangle.height())));
    }

    painter->drawPixmap(rectangle.x(),rectangle.y(),*background);
}

/*!
  \internal
  \fn void SelectedItem::paint(QPainter *painter,const QStyleOptionGraphicsItem *option,QWidget *widget)
  Overrides the superclass method. Handles the static display of the SelectedItem, object
  movement and animation. Calls drawBackground(...) and draw(...).
*/
void SelectedItem::paint(QPainter *painter,const QStyleOptionGraphicsItem *,QWidget *)
{
    if ( !currentItem ) {
        qWarning("SelectedItem::paint(...): No current item.");
        return;
    }

    // Draw the background.
    drawBackground(painter);

    if ( animationState() == Animating ) {
        // During animation, we get multiple calls to paint(...). In each, we paint the next
        // frame of the movie.
        if ( movie ) {
            QPixmap moviePixmap = movie->currentPixmap();
            draw(painter,moviePixmap,
                    static_cast<int>(rect().x()),static_cast<int>(rect().y()));
        } else {
            // We'll try to do a manual animation using the GridItem's Animator object.
            // This call to paint(...) will have been invoked via a signal from playTimeLine
            // which ultimatey triggers playStep(...), which saves the point at which the
            // animation has run to (animationStage) - drawAnimated(...) will make use of
            // animationStage.
            drawAnimated(painter);
        }
    } else {
        // Not currently animating, but we may be sliding across from one item to its
        // neighbour.
        if ( destItem ) {
            // Yes, moving across -- we're going to draw selected images for both the
            // source item and the destination item, but we're going to use this item as
            // the clipping region. First, set up the clipping region.
            painter->setClipRect(rect());

            // During moveStep(...), we calculated new positions for the two items when they
            // are drawn as SelectedItems. This calculation takes into account the magnified
            // area between the two pixmaps.
            draw(painter,currentItem->selectedPic(),currentX,currentY);
            draw(painter,destItem->selectedPic(),destX,destY);
        } else {
            // We're not sliding, we're just drawing 'item' as the selected item.
            draw(painter,currentItem->selectedPic(),static_cast<int>(rect().x()),static_cast<int>(rect().y()));
        }
    }
}

/*!
  \internal
  \fn void SelectedItem::draw(QPainter *painter,const QPixmap &pixmap,int x,int y) const
  Draws the given pixmap in the centre of an area where \a x and \a y indicate the
  top left-hand corner of the area. If this SelectedItem is 'active', draws the pixmap
  with a different appearance.
*/
void SelectedItem::draw(QPainter *painter,const QPixmap &pixmap,int x,int y) const
{
    if ( !pixmap.isNull() ) {
        // Position the image in the centre of this item.
        x += static_cast<int>(rect().width()/2.0 - pixmap.width()/2.0);
        y += static_cast<int>(rect().height()/2.0 - pixmap.height()/2.0);

        // The 'active' image has a different appearance.
        if ( active ) {
            QImage img = pixmap.toImage();
            blendColor(img,highlightColor);
            painter->drawPixmap(x,y,QPixmap::fromImage(img));
        } else {
            painter->drawPixmap(x,y,pixmap);
        }
    }
}

/*!
  \internal
  \fn void SelectedItem::drawAnimated(QPainter *painter)
  Draws a single step of the animation for a manual animation, using the current
  GridItem's Animator object.
*/
void SelectedItem::drawAnimated(QPainter *painter)
{
    // Pass painter and the percentage of the animation through to the current Animator.
    Animator *animator = currentItem->getSelectedAnimator();
    if ( !animator ) {
        draw(painter,currentItem->selectedPic(),static_cast<int>(rect().x()),static_cast<int>(rect().y()));
        return;
    }

    // Finds how far we are through the animation - 'percent' will be a value between
    // 0 and 1.
    qreal percent = static_cast<qreal>(animationStage)/100;
    // Draw the animated background first - if there is one - then the animator for
    // this stage of the animation.
    Animator *backgroundAnimator = currentItem->getSelectedBackgroundAnimator();
    painter->setBrush(backgroundHighlightColor);
    if ( backgroundAnimator ) {
        backgroundAnimator->animate(painter,this,percent);
    }
    animator->animate(painter,this,percent);
}

/*!
  \internal
  \fn void SelectedItem::blendColor(QImage &img,QColor color)
  Used when this SelectedItem is 'active'.
*/
void SelectedItem::blendColor(QImage &img,QColor color)
{
    //if (img.format() == QImage::Format_ARGB32_Premultiplied) {
    //    img = img.convertToFormat(QImage::Format_ARGB32);
    //}
    color.setAlphaF(0.4);

    // Blend the pixels in 'img' with those in 'color'.
    QPainter blendPainter(&img);
    //blendPainter.setRenderHint(QPainter::Antialiasing,true);
    blendPainter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    blendPainter.fillRect(0,0,img.width(),img.height(),color);
    blendPainter.end();
}

/*!
  \internal
  \fn void SelectedItem::detachAnimation()
  Stops animation and disconnects the current movie.
*/
void SelectedItem::detachAnimation()
{
    stopAnimation();

    // If there was a movie, make sure it isn't still connected to any slots that we're responsible
    // for, and kiss it goodbye (but do not delete it - it is owned by a GridItem).
    if ( movie ) {
        QObject::disconnect(movie,SIGNAL(frameChanged(int)),connector,SLOT(animationChanged()));
        QObject::disconnect(movie,SIGNAL(finished()),connector,SLOT(animationFinished()));
        QObject::disconnect(movie,SIGNAL(error(QImageReader::ImageReaderError)),connector,SLOT(animationError(QImageReader::ImageReaderError)));
        movie = 0;
    }
}

/*!
  \internal
  \fn void SelectedItem::resetAnimation()
  Stops and disconnects the current animation, fetches new animation information from
  the current GridItem object and connects it up to this object's connector.
*/
void SelectedItem::resetAnimation()
{
    // Stop and disconnect any current movie.
    detachAnimation();

    if ( !currentItem ) {
        // Nothing now selected.
        return;
    }

    // Set up the movie for the new current item.
    movie = currentItem->movie();
    if ( movie ) {
        // Item can animate.
        QObject::connect(movie,SIGNAL(frameChanged(int)),connector,SLOT(animationChanged()));
        QObject::connect(movie,SIGNAL(finished()),connector,SLOT(animationFinished()));
        QObject::connect(movie,SIGNAL(error(QImageReader::ImageReaderError)),connector,SLOT(animationError(QImageReader::ImageReaderError)));
    }
}

/*!
  \internal
  \fn void SelectedItem::updateImages()
  Updates all the Pixmaps for all the GridItem objects.
*/
void SelectedItem::updateImages()
{
    // The images have changed. First, get the SelectedItem to disconnect from the movie.
    detachAnimation();

    // Get all the GridItem objects to rebuild their pixmaps.
    for ( int row = 0; row < table.getTopRow(); row++ ) {
        for ( int col = 0; col < table.getTopColumn(); col++ ) {
            GridItem *item = table.getItem(row,col);
            if ( item ) {
                item->updateImages();
            }
        }
    }

    // SelectedItem now has to grab the revised movie.
    resetAnimation();
}

/*!
  \internal
  \fn void SelectedItem::resize()
  Should be called in response to a resize event. Resizes this object with respect to the
  current item. If there is no current item, does nothing.
*/
void SelectedItem::resize()
{
    selectedSize = QSize(-1,-1);

    if ( currentItem ) {
        // Position this object over currentItem.
        QRect rectangle = getGeometry(currentItem);
        setRect(rectangle);
    }
}

/*!
  \internal
  \fn void SelectedItem::setActive(bool isActive)
  Modifies the \a active attribute, which affects the way that this object is drawn.
*/
void SelectedItem::setActive(bool isActive)
{
    if ( isActive == active ) {
        return;
    }

    active = isActive;

    // Get it to redraw (active/inactive causes a change in appearance).
    update();
}

/*!
  \internal
  \fn void SelectedItem::setCurrent(GridItem *gridItem,bool animate = true);
  Changes the current GridItem object, and causes it to start animating. The item may be 0,
  indicating that no item is currently selected. Emits the selectionChanged signal.
*/
void SelectedItem::setCurrent(GridItem *item,bool animate)
{
    currentItem = item;

    resetAnimation();

    // Resize this object.
    resize();

    // Now we are current - tell the world.
    connector->triggerSelectionChanged(currentItem);

    if ( currentItem && animate ) {
        // When a new item is current (i.e. a move is finished, or the view has been shown) we animate
        // the SelectdItem.
        startAnimating();
    }
}

/*!
  \internal
  \fn void SelectedItem::setCurrent(int row,int column,bool animate);
  Changes the current GridItem object by calling \l{function}{SelectedItem::setCurrent(GridItem *item)}.
*/
void SelectedItem::setCurrent(int row,int column,bool animate)
{
    GridItem *item = table.getItem(row,column);
    setCurrent(item,animate);
}

/*!
  \internal
  \fn void SelectedItem::keyPressEvent(QKeyEvent *event)
  Calls \l{function}{moveRequested(Direction)} in response to Qt::Key_Right, Qt::Key_Left,
  Qt::Key_Up or Qt::Key_Down. Calls triggerItemSelected() in response to Qt::Key_Select or
  Qt::Key_Return. For all other keys, the default functionality applies.
  If there is no current item, does nothing.
*/
void SelectedItem::keyPressEvent(QKeyEvent *event)
{
    if ( event->isAutoRepeat() ) {
        return;
    }
    if ( !currentItem ) {
        qWarning("SelectedItem::keyPressEvent(...): No current item.");
        QGraphicsRectItem::keyPressEvent(event);
        return;
    }

    switch( event->key() ) {
    case Qt::Key_Right:
        moveRequested(Right);
        break;
    case Qt::Key_Left:
        moveRequested(Left);
        break;
    case Qt::Key_Up:
        moveRequested(Up);
        break;
    case Qt::Key_Down:
        moveRequested(Down);
        break;
    case Qt::Key_Select: // Qtopia pad
    case Qt::Key_Return: // otherwise
        if ( destItem ) {
            // User has chosen to move to a new destination already, so they want to select
            // that one.
            triggerItemSelected(destItem);
        } else if ( currentItem ) {
            triggerItemSelected(currentItem);
        } // Otherwise, nothing to select
        break;
    default:
        QGraphicsRectItem::keyPressEvent(event);
        return;
    }
}

/*!
  \internal
  \fn void SelectedItem::moveRequested(Direction direction)
  Moves this object in the given direction. Calls \l{function}{moveTo(GridItem *_destItem)}.
*/
void SelectedItem::moveRequested(Direction direction)
{
    int row;
    int col;
    if ( destItem ) {
        // Move already in progress.
        row = destItem->getRow();
        col = destItem->getColumn();
    } else {
        row = currentItem->getRow();
        col = currentItem->getColumn();
    }

    switch( direction) {
    case Right:
        col++;
        break;
    case Left:
        col--;
        break;
    case Up:
        row--;
        break;
    case Down:
        row++;
        break;
    default:
        qWarning("SelectedItem::moveRequested(...): Error - invalid direction of %d.",direction);
        return;
    }

    if ( (row < 0) || (row > table.getTopRow()) || (col < 0) || (col > table.getTopColumn()) ) {
        // We ain't going nowhere.
        return;
    }

    // Retrieve the destination item.
    GridItem *_destItem = table.getItem(row,col);
    if ( !_destItem ) {
        qWarning("SelectedItem::moveRequested(...): Error - no item for row %d, column %d.",row,col);
        return;
    }

    // Move the selection.
    moveTo(_destItem);
}

/*!
  \internal
  \fn void SelectedItem::triggerItemSelected(GridItem *)
  Called when an item has been selected/invoked.
  Emits SelectedItemConnector::itemSelected() signal.
*/
void SelectedItem::triggerItemSelected(GridItem *item)
{
    if ( isAnimating() ) {
        stopAnimation();
    }

    // Cause connector to emit itemSelected(GridItem *) signal.
    connector->triggerItemSelected(item);
}

qreal SelectedItem::getXDrift()
{
    if ( xDrift == -1 ) {
        // Hasn't been worked out yet.
        xDrift = ((1 + GridItem::SELECTED_IMAGE_SIZE_FACTOR) * rect().width())
                 - rect().width();
    }

    return xDrift;
}

qreal SelectedItem::getYDrift()
{
    if ( yDrift == -1 ) {
        // Hasn't been worked out yet.
        yDrift = ((1 + GridItem::SELECTED_IMAGE_SIZE_FACTOR) * rect().height())
                 - rect().height();
    }

    return yDrift;
}

/*!
  \internal
  \fn void SelectedItem::moveTo(GridItem *_destItem)
  Positions the SelectedItem over the given GridItem, by starting the timer that will
  gradually shift this SelectedItem. The timer will cause moveStep(int) to be
  called, periodically.
*/
void SelectedItem::moveTo(GridItem *_destItem)
{
    // Stop any previous move and any animations.
    if ( destItem ) {
        // Setting destItem to 0 means that setCurrent(...) won't be called by moveFinished,
        // which would be overkill, as it respositions the object, restarts animation, etc.
        // What WAS the destination item becomes the new current item.
        currentItem = destItem;
        destItem = 0;
    }
    if ( moveTimeLine->state() != QTimeLine::NotRunning ) {
        moveTimeLine->stop();  // triggers moveFinished
        // TODO: This is because Qt's QTimeLine has a bug which doesn't reset the frames
        // when you stop the timeline. When you restart, it acts as though it has been paused.
        // Repair when Qt bug is resolved.
        //createMoveTimeLine();
    }

    // The order is important here. It is possible that this move request is interrupting an
    // animation. If that's the case, we'll want to stop the animation - BUT that will trigger
    // animationFinished() which calls for a redraw that we don't actually want. We set destItem
    // FIRST (so it's non-zero), so that animationFinished() can check if there's a move in the
    // pipeline, and refrain from drawing.
    destItem = _destItem;

    if ( isAnimating() ) {
        stopAnimation(); // triggers animationFinished()
    }

    // Start the timer. This will cause moveStep(int) to be called, periodically.
    moveTimeLine->start();
}

/*!
  \internal
  \fn void SelectedItem::moveStep(int percent)
  Called periodically during a move operation. Moves this object between currentItem and
  destItem.
*/
void SelectedItem::moveStep(int n)
{
    qreal percent = n/100.0;

    // Sanity check.
    if ( !currentItem || !destItem ) {
        qWarning("SelectedItem::moveStep(...): Error - either current item missing or destination item missing.");
        return;
    }

    // Find out how where the SelectedItem box ought to be at this stage in the move, and
    // shift it accordingly.
    QRect destRect = getGeometry(destItem);
    QRect srcRect = getGeometry(currentItem); // we might be in the middle of a move, so don't use rect() here!

    qreal moveByX = (destRect.x() - srcRect.x()) * percent;
    qreal moveByY = (destRect.y() - srcRect.y()) * percent;

    if ( isAnimating() ) {
        // THis should not be happening, as we stop the movie as soon as a move starts.
        qWarning("SelectedItem::moveStep(...): Still animating when it should not be.");
        stopAnimation();
    }

    setRect(srcRect.x() + moveByX,srcRect.y() + moveByY,srcRect.width(),srcRect.height());

    // We also have to find where the 2 images are going to sit within their designated areas,
    // because SelectedItem is essentially a magnified version of an underlying GridItem, and
    // when we're moving between two, the area between them is magnified too, i.e. the dest
    // item will move TOWARDS its usual position, while the current (soon-to-be-old) item will
    // move AWAY FROM its usual position.

    // Find the "ordinary" positions of the two items, i.e. the top left-hand corners
    // of the 2 items when they are magnified as SelectedItems.
    currentX = srcRect.x();
    currentY = srcRect.y();
    destX = destRect.x();
    destY = destRect.y();

    // Find the x position of current and dest items.
    if ( currentItem->getColumn() < destItem->getColumn() ) {
        // Moving from left to right.
        currentX -= qRound(percent * getXDrift()); // current (left) drifts away to the left over time
        destX += qRound((1-percent) * getXDrift()); // destination (right) drifts in from the right over time
    } else if ( currentItem->getColumn() > destItem->getColumn() ) {
        // Moving from right to left.
        currentX += qRound(percent * getXDrift()); // current (right) drifts away to the right over time
        destX -= qRound((1-percent) * getXDrift()); // destination (left) drifts in from the left over time
    } // else, if they're equal, currentX & destX will be their usual positions

    // Find the y position of current and dest items.
    if ( currentItem->getRow() < destItem->getRow() ) {
        // Moving down.
        currentY -= qRound(percent * getYDrift()); // current (top) drifts upwards over time
        destY += qRound((1-percent) * getYDrift()); // destination (bottom) drifts up from below over time
    } else if ( currentItem->getRow() > destItem->getRow() ) {
        // Moving up.
        currentY += qRound(percent * getYDrift()); // current (bottom) drifts downwards over time
        destY -= qRound((1-percent) * getYDrift()); // destination (top) drifts down from above over time
    } // else, if they're equal, currentY & destY will be their usual positions
}

/*!
  \internal
  \fn void SelectedItem::moveFinished()
  Called when timeline's state changed to NotRunning. Calls \l{function}{setCurrent(GridItem *)}
  and switches off \a destItem.
*/
void SelectedItem::moveFinished()
{
    if ( destItem ) {
        setCurrent(destItem);
    }

    destItem = 0;
}

/*!
  \internal
  \fn void SelectedItem::playStep(int _animationStage)
  Called during manual animation (i.e. when no movie is available).
  Causes the item to be redrawn for this step of the animation.
*/
void SelectedItem::playStep(int _animationStage)
{
    animationStage = _animationStage;

    update(boundingRect());
}

SelectedItem::AnimationState SelectedItem::animationState() const
{
    if ( movie && (movie->state() == QMovie::Running) ) {
        return Animating;
    }
    if ( playTimeLine && (playTimeLine->state() == QTimeLine::Running) ) {
        return Animating;
    }
    if ( playTimeLine && animationPending ) {
        return AnimationPending;
    }
    return NotAnimating;
}

/*!
  \internal
  Returns true if the item is currently animating or about to animate.
*/
bool SelectedItem::isAnimating() const
{
    AnimationState state = animationState();
    return ( state == Animating || state == AnimationPending );
}

/*!
  \internal
  \fn void SelectedItem::stopAnimation()
*/
void SelectedItem::stopAnimation()
{
    // Theoretically, these should not both be running at the same time. But you should never
    // trust the wayward devices of the Dark Hand of maintenance...
    if ( movie ) {
        movie->stop();
    }
    if ( playTimeLine ) {
        animationPending = false;
        playTimeLine->stop(); // triggers animationFinished()
        // TODO: This is because Qt's QTimeLine has a bug which doesn't reset the frames
        // when you stop the timeline. When you restart, it acts as though it has been paused.
        // Repair when Qt bug is resolved.
        createPlayTimeLine();
    }
}

/*!
  \internal
  \fn void SelectedItem::startAnimating()
   Delays for a short interval (ANIMATION_DELAY_INTERVAL) to avoid animting when the user is
   quickly moving between objects, then starts the animation timeline.
*/
void SelectedItem::startAnimating()
{
    // Delay animation, so that if users are quickly moving around icons with the arrow keys,
    // they won't be annoyed by weird effects caused by animations starting up and then
    // immediately stopping, which also affects performance.
    animationPending = true; // I hate these stupid little flags, but it gets around the problem that
    // we're delaying an event and we can find out about it in the meantime.
    QTimer::singleShot(ANIMATION_DELAY_INTERVAL,connector,SLOT(startAnimation()));
}

void SelectedItem::startAnimationDelayed()
{
    if ( movie ) {
        // Start the movie -- this will advance frames so you will get signals to draw.
        // If the movie is already running, this function does nothing.
        movie->start();
    } else {
        if ( !animationPending ) {
            // Animation was prematurely halted.
            return;
        }
        animationPending = false; // not pending anymore, we're doing it
        if ( !currentItem ) {
            qWarning("SelectedItem::startAnimating() - trying to start animation, but no current item!");
            return;
        }
        if ( currentItem->getSelectedAnimator() ) {
            // We can start animating manually.
            playTimeLine->start();
        }
        // Otherwise, no animation for this item.
    }
}

/*!
  \internal
  \fn void SelectedItem::animationFinished()
  Refreshes this item when the animation is done, unless a move request has interrupted
  the animation, in which case this method does nothing and lets the move handle drawing
  from now on.
*/
void SelectedItem::animationFinished()
{
    // We only want to do a redraw IF a move is not already in progress. This is because
    // a move request can interrupt an animation. The move request resets values such as
    // currentX and Y, so if we try to draw to finish off the animation we'll get bogus values.
    // We let the move callbacks take care of it, instead.
    if ( !destItem ) {
        // No move in progress, so redraw.
       update(boundingRect());
    }
}
