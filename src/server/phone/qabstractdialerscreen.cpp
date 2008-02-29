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

#include "qabstractdialerscreen.h"

/*!
  \class QAbstractDialerScreen
  \group QtopiaServer::PhoneUI::TTSmartPhone
  \brief The QAbstractDialerScreen class allows developers to replace the "dialer screen" portion of the Phone UI.

  The QAbstractDialerScreen interface allows developers to replace the standard
  touchscreen or quick dialers in Qtopia's phone UI.
 */

/*!
  \fn QAbstractDialerScreen::QAbstractDialerScreen(QWidget *parent = 0, Qt::WFlags flags = 0)

  Construct a new QAbstractDialerScreen instance, with the specified \a parent
  and widget \a flags.
 */

/*!
  \fn QString QAbstractDialerScreen::digits() const

  Returns the currently displayed number from the dialer screen.
 */

/*!
  \fn void QAbstractDialerScreen::reset()

  Reset any displayed number and return to the screens initial view.  This is
  always called before displaying the screen to the user.
 */

/*!
  \fn void QAbstractDialerScreen::appendDigits(const QString &digits)

  Append \a digits to the current number.
 */

/*!
  \fn void QAbstractDialerScreen::setDigits(const QString &digits)

  Set the current number to  \a digits.
 */

/*!
  \fn void QAbstractDialerScreen::requestDial(const QString &number, const QUniqueId &contact)

  Emitted whenever the user has selected a \a number to dial from the dial
  screen.  If valid, \a contact may be set to the unique id of the contact to
  which the number refers.  The dialer is generally expected to QWidget::close()
  after emitting this signal.
 */

/*!
  \fn void QAbstractDialerScreen::speedDial(const QString &number)

  Emitted whenever the user speed dials a number from the dial screen.  Qtopia
  will resolve the \a number into a speed dial action and perform it
  appropriately.
 */

/*!
  \fn void QAbstractDialerScreen::filterKeys(const QString& input, bool& filtered)

  Emitted whenever a key is entered which modified the \a input.
  Slots connected to this signal may elect to filter out the input
  and handle it themselves.  If they do, \a filtered should be
  set to true.
*/

/*!
  \fn void QAbstractDialerScreen::filterSelect(const QString& input, bool& filtered)

  Emitted when the user selects the \a input within the dialer,
  usually by pressing the call button.  Slots connected to this signal
  may elect to filter out the input and handle it themselves.
  If they do, \a filtered should be set to true.
*/
