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

#include <qpixmap.h>
#include <qstring.h>
#include "qdevicebutton.h"

/*! \class QDeviceButton
    \brief The QDeviceButton class represents a user-programmable button on a Qtopia device.

    This class represents a physical special-function button on a Qtopia device.
    The location and number of buttons will vary from device to
    device and the button is described in the help documentation by \c userText() and \c pixmap().

    \ingroup userinput
*/

/*!
  Constructs QDeviceButton.
*/
QDeviceButton::QDeviceButton() :
    m_Keycode(0),
    m_PressedActionMapable(true),
    m_HeldActionMapable(true)
{
}

/*!
  Destructs QDeviceButton.
*/
QDeviceButton::~QDeviceButton()
{
}

/*!
  Returns the button keycode.
 */
int QDeviceButton::keycode() const
{
    return m_Keycode;
}


/*!
  Returns the button context. If the context is empty, then the button
  applies in all contexts. The semantics of non-empty contexts is device specific.
 */
QString QDeviceButton::context() const
{
    return m_Context;
}


/*!
  Returns a human-readable, translated description of the button.
 */
QString QDeviceButton::userText() const
{
    return m_UserText;
}

/*!
  Returns the pixmap for this button.    If there isn't one
  it will return an empty (null) pixmap.
 */
QPixmap QDeviceButton::pixmap() const
{
    if ( m_Pixmap.isNull() && !m_PixmapName.isEmpty() ) {
        QDeviceButton *that = (QDeviceButton *)this;
        that->m_Pixmap = QPixmap( ":image/"+m_PixmapName );
    }
    return m_Pixmap;
}

/*!
  Returns the user-assigned action for when this button is pressed.
 */
QtopiaServiceRequest QDeviceButton::pressedAction() const
{
    return m_PressedAction;
}

/*!
  Returns the user-assigned action for when this button is pressed
  and held.
 */
QtopiaServiceRequest QDeviceButton::heldAction() const
{
    return m_HeldAction;
}

/*!
  Sets the \a keycode that is sent when the button is pressed.
*/
void QDeviceButton::setKeycode(int keycode)
{
    m_Keycode = keycode;
}

/*!
  Sets the \a context where the keycode applies.
*/
void QDeviceButton::setContext(const QString& context)
{
    m_Context = context;
}

/*!
  Sets the human-readable, translated description of the button to \a text.
*/
void QDeviceButton::setUserText(const QString& text)
{
    m_UserText = text;
}

/*!
  Set the pixmap for this button to the resource named \a pmn.
  This will ideally match the label on the physical button.
*/
void QDeviceButton::setPixmap(const QString& pmn)
{
    if ( !m_PixmapName.isEmpty() )
        m_Pixmap = QPixmap();
    m_PixmapName = pmn;
}

/*!
  Set the action to be performed when this button is pressed to \a action.
*/
void QDeviceButton::setPressedAction(const QtopiaServiceRequest& action)
{
    m_PressedAction = action;
}

/*!
  Set the action to be performed when this button is pressed and
  held to \a action.
*/
void QDeviceButton::setHeldAction(const QtopiaServiceRequest& action)
{
    m_HeldAction = action;
}

/*!
  \fn bool QDeviceButton::pressedActionMappable() const
  Returns the mappability set for the pressed action.
*/

/*!
  \fn bool QDeviceButton::heldActionMappable() const
  Returns the mappability set for the pressed action.
*/

/*!
  Set the mappability of the pressed action to \a mappable.
*/
void QDeviceButton::setPressedActionMappable(bool mappable)
{
    m_PressedActionMapable = mappable;
}

/*!
  Set the mappability of the held action to \a mappable.
*/
void QDeviceButton::setHeldActionMappable(bool mappable)
{
    m_HeldActionMapable = mappable;
}


/*!
  Returns true if this QDeviceButton is equal to \a e.
*/
bool QDeviceButton::operator==(const QDeviceButton &e) const
{
    return ((keycode() == e.keycode()) &&
            (userText() == e.userText()) &&
            (context() == e.context()) &&
            (pressedAction() == e.pressedAction()) &&
            (heldAction() == e.heldAction()));
}
