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

#include "selecteditemconnector.h"
#include "selecteditem.h"

#include <QDebug>


/*!
  \internal
  \class SelectedItemConnector

  \brief Class to handle signals/slots on behalf of SelectedItem, while avoiding multiple
  inheritance.

  \sa SelectedItem
*/


/*!
  \internal
  \fn SelectedItemConnector::SelectedItemConnector(SelectedItem *_selectedItem)
*/
SelectedItemConnector::SelectedItemConnector(SelectedItem *_selectedItem)
    : QObject()
    , selectedItem(_selectedItem)
{
}

/*!
  \internal
  \fn void SelectedItemConnector::itemSelected(GridItem *)
  The given GridItem object has been "invoked".
*/

/*!
  \internal
  \fn void SelectedItemConnector::selectionChanged(GridItem *)
  The SelectedItem has moved its position to highlight the given GridItem object.
*/

/*!
  \internal
  \fn void SelectedItemConnector::triggerSelectionChanged(GridItem *item)
  Triggers the \l{function}{selectionChanged(GridItem *)} signal.
*/
void SelectedItemConnector::triggerSelectionChanged(GridItem *item)
{
    emit selectionChanged(item);
}

/*!
  \internal
  \fn void SelectedItemConnector::moving(int n)
  Calls \l{function}{SelectedItem::moveStep(int)}.
*/
void SelectedItemConnector::moving(int n)
{
    selectedItem->moveStep(n);
}

/*!
  \internal
  \fn void SelectedItemConnector::movingStateChanged(QTimeLine::State state)
  When \a state has changed to QTimeLine::NotRunning, tells the SelectedItem to complete
  its move.
*/
void SelectedItemConnector::movingStateChanged(QTimeLine::State state)
{
    // We're only interested in when the timeline finishes -- at that point,
    // we can change the selected item.
    if ( state == QTimeLine::NotRunning ) {
        selectedItem->moveFinished();
    }
}

void SelectedItemConnector::startAnimation()
{
    selectedItem->startAnimationDelayed();
}

/*!
  \internal
  \fn void SelectedItemConnector::animationStateChanged(QTimeLine::State state)
  When \a state has changed to QTimeLine::NotRunning, repaint.
*/
void SelectedItemConnector::animationStateChanged(QTimeLine::State state)
{
    // We're only interested in when the timeline finishes.
    if ( state == QTimeLine::NotRunning ) {
        selectedItem->animationFinished();
    }
}

/*!
  \internal
  \fn void SelectedItemConnector::animationChanged()
  Updates the SelectedItem (i.e. causes a redraw).
*/
void SelectedItemConnector::animationChanged()
{
    // Another frame change in the animation - just cause the item to redraw itself.
    selectedItem->update(selectedItem->boundingRect());
}

/*!
  \internal
  \fn void SelectedItemConnector::animationFinished()
  Tells the SelectedItem to complete its animation.
*/
void SelectedItemConnector::animationFinished()
{
    selectedItem->animationFinished();
}

/*!
  \internal
  \fn void SelectedItemConnector::animationError(QImageReader::ImageReaderError error)
*/
void SelectedItemConnector::animationError(QImageReader::ImageReaderError error)
{
    qWarning() << "SelectedItemConnector::animationError: " << error;
}

/*!
  \internal
  \fn void SelectedItemConnector::triggerItemSelected(GridItem *item)
  Triggers the \l{function}{SelectedItemConnector::triggerItemSelected(GridItem *item)} signal.
*/
void SelectedItemConnector::triggerItemSelected(GridItem *item)
{
    emit itemSelected(item);
}

/*!
  \internal
  \fn void SelectedItemConnector::playing(int n)
  Used during manual animations, i.e. in response to a QTimeLine.
  Calls \l{function}{SelectedItem::animateStep(int)}.
*/
void SelectedItemConnector::playing(int n)
{
    // Another frame change in the animation - just cause the item to redraw itself.
    selectedItem->playStep(n);
}
