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

#include "devicebutton.h"
#include "devicebuttonmanager.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/resource.h>
#include <qtopia/services.h>

#include <qaccel.h>
#include <qobject.h>
#include <qvaluelist.h>

QString qtopia_internal_defaultButtonsFile();

DeviceButtonManager* DeviceButtonManager::m_Instance = 0L;

/*!
  \class DeviceButtonManager devicebuttonmanager.h
  \brief The DeviceButtonManager class manages device button mappings.

  The function buttons on a device may generate key presses and also
  activate services.  DeviceButtonManager allows the role of the function
  buttons to be accessed or modified.

  First availability: Qtopia 1.6

  \ingroup qtopiaemb
*/

/*!
  Returns an instance of the DeviceButtonManager.  DeviceButtonManager
  should never be constructed explicitly.
*/
DeviceButtonManager& DeviceButtonManager::instance()
{
    if (!m_Instance)
	m_Instance = new DeviceButtonManager();
    return *m_Instance;
}

/*!
  \internal
*/
DeviceButtonManager::DeviceButtonManager() : QObject(0L)
{
#ifndef QT_NO_COP
  QCopChannel *channel = new QCopChannel( "QPE/System", this );
  connect( channel, SIGNAL(received(const QCString&,const QByteArray&)),
	   this, SLOT(received(const QCString&,const QByteArray&)) );
#endif
  loadButtons();
}

/*!
  \internal
*/
DeviceButtonManager::~DeviceButtonManager()
{
}

/*!
 Returns the available buttons on this device.  The number and location
 of buttons will vary depending on the device.  Button numbers will be assigned
 by the device manufacturer and will be from most preferred button to least preffered
 button.  Note that this list only contains "user mapable" buttons.
 */
const QValueList<DeviceButton>& DeviceButtonManager::buttons() const
{
  return m_Buttons;
}


/*!
  Returns the DeviceButton for the \a keyCode.  If \a keyCode is not found, it
  returns 0L
 */
const DeviceButton* DeviceButtonManager::buttonForKeycode(ushort keyCode)
{
    QValueListConstIterator<DeviceButton> it;
    for (it = buttons().begin(); it != buttons().end(); ++it)
	if ((*it).keycode() == keyCode)
	    return &(*it);
    return 0L;
}

/*!
 Reassigns the pressed action for \a button_index to \a action.
 */
void DeviceButtonManager::remapPressedAction(int button_index, const ServiceRequest& action)
{
    DeviceButton& button = m_Buttons[button_index];
    button.setPressedAction(action);
    Config buttonFile("ButtonSettings");
    buttonFile.setGroup("Button" + QString::number(button_index));
    buttonFile.writeEntry("PressedActionService", (const char*)button.pressedAction().service());
    buttonFile.writeEntry("PressedActionMessage", (const char*)button.pressedAction().message());
    buttonFile.writeEntry("PressedActionArgs", button.pressedAction().data());
#ifndef QT_NO_COP
    QCopEnvelope("QPE/System", "deviceButtonMappingChanged()");
#endif
}

/*!
 Reassigns the held action for \a button_index to \a action. 
 */
void DeviceButtonManager::remapHeldAction(int button_index, const ServiceRequest& action)
{
    DeviceButton& button = m_Buttons[button_index];
    button.setHeldAction(action);
    Config buttonFile("ButtonSettings");
    buttonFile.setGroup("Button" + QString::number(button_index));
    buttonFile.writeEntry("HeldActionService", (const char*)button.heldAction().service());
    buttonFile.writeEntry("HeldActionMessage", (const char*)button.heldAction().message());
    buttonFile.writeEntry("HeldActionArgs", button.heldAction().data());
#ifndef QT_NO_COP
    QCopEnvelope("QPE/System", "deviceButtonMappingChanged()");
#endif
}

void DeviceButtonManager::loadButtons(bool factory)
{
    m_Buttons.clear();

    for (int local=0; local<2; local++) {
	Config buttonFile(
	    local ? QString("ButtonSettings") : qtopia_internal_defaultButtonsFile(),
	    local ? Config::User : Config::File);
	if (!buttonFile.isValid())
	    continue;
	QString groupBase("Button"); // No tr
	int n;
	if ( local ) {
	    n = m_Buttons.count();
	} else {
	    buttonFile.setGroup(groupBase);
	    n = buttonFile.readNumEntry("Count",0);
	    DeviceButton blank;
	    for (int i=0; i<n; i++)
		m_Buttons.append(blank);
	}
	int i=0;
	for (QValueListIterator<DeviceButton> it = m_Buttons.begin(); i<n; ++it,++i) {
	    QString groupName = groupBase + QString::number(i);
	    buttonFile.setGroup(groupName);
	    if ( factory && local ) {
		buttonFile.clearGroup();
	    } else {
		DeviceButton& curButton = (*it);
		ServiceRequest pressedAction(buttonFile.readEntry("PressedActionService").latin1(), buttonFile.readEntry("PressedActionMessage").latin1());
		QByteArray tempArray = buttonFile.readByteArrayEntry("PressedActionArgs");
		if (!tempArray.isEmpty()) {
		    pressedAction.writeRawBytes(tempArray.data(), tempArray.size());
		}
		curButton.setPressedAction(pressedAction);

		ServiceRequest heldAction(buttonFile.readEntry("HeldActionService").latin1(), buttonFile.readEntry("HeldActionMessage").latin1());
		tempArray = buttonFile.readByteArrayEntry("HeldActionArgs");
		if (!tempArray.isEmpty()) {
		  heldAction.writeRawBytes(tempArray.data(), tempArray.size());
		}
		curButton.setHeldAction(heldAction);

		if ( !local ) {
		    // non-variable values
		    curButton.setKeycode(QAccel::stringToKey(buttonFile.readEntry("Key")));
		    curButton.setUserText(buttonFile.readEntry("Name"));
		    curButton.setPixmap(QString("Button/%1.png").arg(i));
		}
	    }
	}
    }
}

void DeviceButtonManager::received(const QCString& message, const QByteArray&)
{
    if (message == "deviceButtonMappingChanged()")
	loadButtons();
}

/*!
  Reset the buttons back to the factory default settings.
*/
void DeviceButtonManager::factoryResetButtons()
{
    loadButtons(TRUE);
}
