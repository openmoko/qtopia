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

/*!
  If \a b is true sets the dialog \a diag to have Menu Like behaviour when executed.
  Otherwise sets dialog to have default dialog behaviour.

  Menu Like dialogs typically have a single list of options, and
  should accept the dialog when the select key is pressed on the appropriate item,
  and when a mouse/stylus is used to click on an item - just like menus.
  Menu Like dialogs should only have one widget accepting key focus.
  
  By marking a dialog as Menu Like Qtopia will map the Back key to reject the dialog
  and will not map any key to accept the dialog - you must do that yourself.

  The default dialog behaviour is to include a cancel menu option in the context
  menu to reject the dialog and to map the Back key to accept the dialog.

  Only modal dialogs can be Menu Like.

  \sa isMenuLike()
*/
void QPEApplication::setMenuLike( QDialog *diag, bool b )
{
    if (b == isMenuLike(diag))
	return;

    if (!diag->testWFlags(WType_Modal)) {
	qWarning("Cannot setMenuLike for non-modal dialog: %s (%s)",
	    diag->className(), diag->name() ? diag->name() : "unnamed");
	return;
    }

#ifdef QTOPIA_PHONE
    // can't do here, if QDialog is const..... but it doesn't need to be.
    if (b)
	ContextBar::setLabel(diag, Qt::Key_Back, ContextBar::Cancel);
    else
	ContextBar::setLabel(diag, Qt::Key_Back, ContextBar::Back);

    if (qpeWidgetFlags.contains(diag)) {
	qpeWidgetFlags[diag] = qpeWidgetFlags[diag] ^ MenuLikeDialog;
    } else {
	// the ternery below is actually null.  if b is false, and flags
	// not present, then will never get here, hence b is always
	// true at this point.  Leave in though incase that logic
	// changes.
	qpeWidgetFlags.insert(diag, b ? MenuLikeDialog : 0);
	// connect to destructed signal.
	connect(diag, SIGNAL(destroyed()), qApp, SLOT(removeFromWidgetFlags()));
    }
#endif
}

/*!
  Returns true if the dialog \a diag is set to have Menu Like behaviour when executed.
  Otherwise returns FALSE

  \sa setMenuLike()
*/
bool QPEApplication::isMenuLike( const QDialog *diag)
{
#ifdef QTOPIA_PHONE
    if (qpeWidgetFlags.contains(diag))
	return (qpeWidgetFlags[diag] & MenuLikeDialog == MenuLikeDialog);
#else
    Q_UNUSED(diag); 
#endif
    return FALSE; // default.
}
