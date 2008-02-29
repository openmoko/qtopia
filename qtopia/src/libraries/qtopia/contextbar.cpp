/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "contextbar.h"
#include "contextkeymanager_p.h"

/*!
  \class ContextBar contextbar.h
  \brief The ContextBar class allows the context label in the phone
  context bar to be set.

  ContextBar is only available in the Qtopia phone edition.  It is
  used to set the text in the context bar to describe the action
  performed when pressing the associated context button.  Keep in
  mind that not all phones have soft keys and therefore may not have
  a visible context bar.  The application should be useable without
  soft key shortcuts.

  The context labels are updated whenever a widget gains or loses focus
  or their modal editing state changes.  Therefore, the labels should
  be set for each widget that may gain focus, and will respond to a
  context key.  The standard Qt and Qtopia widgets
  set the labels appropriately, so this is usually only necessary for
  custom widgets.

  The rule for determining what label is displayed on the context bar and
  where the key events are delivered is:

\list
  \i If the current focus widget has claimed the key, then the corresponding
  context label is shown.
  \i Otherwise, the widget's parent, grandparent and so on may set the label.
  \i If the focus widget has not claimed the key and an ancestor has, then
  the key event will be sent directly to the ancestor.
\endlist

  ContextBar labels specify both a pixmap and a text label.  Currently,
  Qtopia Phone Edition will always use the pixmap rather than text.  The
  text label must still be provided because future versions of Qtopia
  Phone Edition may allow the user to choose to view text labels.

  ContextBar is only available in the Qtopia Phone Edition.

  \ingroup qtopiaphone
*/

/*!
  \enum ContextBar::EditingState

  \value Modal apply the label setting when in Modal editing state.
  \value NonModal apply the label setting when in Nonmodal editing state.
  \value ModalAndNonModal apply the label setting regardless of editing state.
*/

/*!
  \enum ContextBar::StandardLabel

  \value NoLabel
  \value Options
  \value Ok
  \value Edit
  \value Select
  \value View
  \value Cancel
  \value Back
  \value BackSpace
  \value Next
  \value Previous
*/


ContextBar::ContextBar()
{
}


/*!
  When widget \a w gains focus, sets the label in the context bar for key
  \a key to text \a t and pixmap \a pm when in state \a state.

  The pixmaps used in the context bar may be no larger than 22x16 pixels.
*/
void ContextBar::setLabel(QWidget *w, int key, const QString &pm, const QString &t, EditingState state)
{
    ContextKeyManager::instance()->setContextText(w, key, t, state);
    ContextKeyManager::instance()->setContextPixmap(w, key, pm, state);
}

/*!
  When widget \a w gains focus, sets the label in the context bar for key
  \a key to the standard label \a label when in state \a state.
*/
void ContextBar::setLabel(QWidget *w, int key, StandardLabel label, EditingState state)
{
    ContextKeyManager::instance()->setContextStandardLabel(w, key, label, state);
}

/*!
  Clears any label set for widget \a w, key \a key in state \a state.

  \sa setLabel()
*/
void ContextBar::clearLabel(QWidget *w, int key, EditingState state)
{
    ContextKeyManager::instance()->clearContextLabel(w, key, state);
}

/*!
  Returns the list of context keys.
*/
const QArray<int> &ContextBar::keys()
{
    return ContextKeyManager::instance()->keys();
}
