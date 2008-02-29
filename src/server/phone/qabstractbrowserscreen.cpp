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

#include "qabstractbrowserscreen.h"

/*!
  \class QAbstractBrowserScreen
  \group QtopiaServer::PhoneUI::TTSmartPhone
  \brief The QAbstractBrowserScreen class allows developers to replace the "application browser screen" portion of the Phone UI.

  The application browser is the portion of the phone UI that users navigate
  through to launch applications or view their documents.
 */

/*!
  \fn QAbstractBrowserScreen::QAbstractBrowserScreen(QWidget *parent, Qt::WFlags flags)

  Construct a new QAbstractBrowserScreen instance, with the specified \a parent
  and widget \a flags.
*/

/*!
  \fn QString QAbstractBrowserScreen::currentView() const

  Returns the name of the current view.
 */

/*!
  \fn bool QAbstractBrowserScreen::viewAvailable(const QString &view) const

  Returns true if the browser is capable of switching to \a view.
 */

/*!
  \fn void QAbstractBrowserScreen::resetToView(const QString &view)

  Display the \a view immediately.  Any logical back or history handling
  should be reset to have \a view as a starting point.
 */

/*!
  \fn void QAbstractBrowserScreen::moveToView(const QString &view)

  Display the \a view using any appropriate transition effects.  \a view
  should be added to the end of any back or history handing list.
 */

/*!
  \fn void QAbstractBrowserScreen::currentViewChanged(const QString &view)

  Emitted whenever the current view changes to \a view.  This may be caused
  by calls the resetToView() or moveToView(), or by user interaction with the
  browser.
 */

/*!
  \fn void QAbstractBrowserScreen::applicationLaunched(const QString &application)

  Emitted whenever the user launches an \a application through the browser.
 */
