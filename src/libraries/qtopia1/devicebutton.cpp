/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qtopia/resource.h>
#include <qpixmap.h>
#include <qstring.h>
#include "devicebutton.h"

/*! \class DeviceButton devicebutton.h
    \brief The DeviceButton class represents a physical user mappable button on a Qtopia device.

    This class represents a physical button on a Qtopia device.    A
    device may have "user programmable" buttons.
    The location and number of buttons will vary from device to
    device.    userText() and pixmap() may be used to describe this button
    to the user in help documentation.

    \ingroup qtopiaemb
*/

/*!
  Constructs DeviceButton.
*/
DeviceButton::DeviceButton()
{
}

/*!
  Destructs DeviceButton.
*/
DeviceButton::~DeviceButton()
{
}

/*!
  Returns the button's keycode.
 */
ushort DeviceButton::keycode() const
{
    return m_Keycode;
}


/*!
  Returns a human readable, translated description of the button.
 */
QString DeviceButton::userText() const
{
    return m_UserText;
}

/*!
  Returns the pixmap for this button.    If there isn't one
  it will return an empty (null) pixmap.
 */
QPixmap DeviceButton::pixmap() const
{
    if ( m_Pixmap.isNull() && !m_PixmapName.isEmpty() ) {
	DeviceButton *that = (DeviceButton *)this;
	that->m_Pixmap = Resource::loadPixmap( m_PixmapName );
    }
    return m_Pixmap;
}

/*!
  Returns the user assigned action for when this button is pressed.
 */
ServiceRequest DeviceButton::pressedAction() const
{
    return m_PressedAction;
}

/*!
  Returns the user assigned action for when this button is pressed
  and held.
 */
ServiceRequest DeviceButton::heldAction() const
{
    return m_HeldAction;
}

/*!
  Sets the keycode \a keycode that is set when the button is pressed.
*/
void DeviceButton::setKeycode(ushort keycode)
{
    m_Keycode = keycode;
}

/*!
  Sets the human readable, translated description of the button.
*/
void DeviceButton::setUserText(const QString& text)
{
    m_UserText = text;
}

/*!
  Set the pixmap for this button.  This will ideally match the label
  on the physical button.
*/
void DeviceButton::setPixmap(const QString& pmn)
{
    if ( !m_PixmapName.isEmpty() )
	m_Pixmap = QPixmap();
    m_PixmapName = pmn;
}

/*!
  Set the action to be performed when this button is pressed to \a action.
*/
void DeviceButton::setPressedAction(const ServiceRequest& action)
{
    m_PressedAction = action;
}

/*!
  Set the action to be performed when this button is pressed and
  held to \a action.
*/
void DeviceButton::setHeldAction(const ServiceRequest& action)
{
    m_HeldAction = action;
}

/*!
  Returns TRUE if this DeviceButton is equal to \a e.
*/
bool DeviceButton::operator==(const DeviceButton &e) const
{
    return ((keycode() == e.keycode()) &&
	    (userText() == e.userText()) &&
	    (pressedAction() == e.pressedAction()) &&
	    (heldAction() == e.heldAction()));
}
