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

#define QTOPIA_INTERNAL_CONFIG_BYTEARRAY

#include "devicebutton.h"
#include "devicebuttonmanager.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/resource.h>
#include <qtopia/services/services.h>

#include <qobject.h>
#include <qvaluelist.h>

struct i_button {
    Qt::Key code;
    char *utext;
    char *pix;
    char *fpressedservice;
    char *fpressedaction;
    char *fheldservice;
    char *fheldaction;
};

#if defined QT_QWS_IPAQ
struct i_button bmap[] = {
    { Qt::Key_F9, QT_TRANSLATE_NOOP("Button", "Calendar Button"), 
	"Button/0.png", 
	"Calendar", "raiseToday()", 
	"Calendar", "raise()" },
    { Qt::Key_F10, QT_TRANSLATE_NOOP("Button", "Contacts Button"), 
	"Button/1.png", 
	"addressbook", "raise()", 
	"addressbook", "beamBusinessCard()" },
    { Qt::Key_F11, QT_TRANSLATE_NOOP("Button", "Mail Button"), 
	"Button/2.png",
	"email", "raise()", 
	"email", "newMail()" },
    { Qt::Key_F12, QT_TRANSLATE_NOOP("Button", "Home Button"), 
	"Button/3.png", 
	"qpe", "raise()", 
	"buttoneditor", "raise()" },
    { Qt::Key_F24, QT_TRANSLATE_NOOP("Button", "Record Button"), 
	"Button/4.png", 
	"recorder", "raise()", 
	"recorder", "startRecording()" },
};
#else
struct i_button bmap[] = {
    { Qt::Key_F9, QT_TRANSLATE_NOOP("Button", "Calendar Button"),
	"Button/0.png",
	"Calendar", "raiseToday()",
	"Calendar", "raise()" },
    { Qt::Key_F10, QT_TRANSLATE_NOOP("Button", "Contacts Button"),
	"Button/1.png",
	"addressbook", "raise()",
	"addressbook", "beamBusinessCard()" },
    { Qt::Key_F13, QT_TRANSLATE_NOOP("Button", "Mail Button"),
	"Button/2.png",
	"email", "raise()",
	"email", "newMail()" },
    { Qt::Key_F12, QT_TRANSLATE_NOOP("Button", "Home Button"),
	"Button/3.png",
	"qpe", "raise()",
	"buttoneditor", "raise()" },
};
#endif

static const int bmapSize = sizeof(bmap)/sizeof(struct i_button);


QValueList<DeviceButton> getCustomDeviceButtons()
{
    QValueList<DeviceButton> buttons;
  
    DeviceButton curButton;

    for (int i = 0; i < bmapSize; i++) {
	curButton.setKeycode(bmap[i].code);
	curButton.setUserText(qApp->translate("Button", bmap[i].utext));
	curButton.setPixmap(bmap[i].pix);
	curButton.setPressedAction(
	    ServiceRequest(bmap[i].fpressedservice, bmap[i].fpressedaction));
	curButton.setHeldAction(
	    ServiceRequest(bmap[i].fheldservice, bmap[i].fheldaction));

	buttons.append(curButton);
    }
    return buttons;
}

DeviceButtonManager* DeviceButtonManager::m_Instance = 0L;

DeviceButtonManager& DeviceButtonManager::instance()
{
    if (!m_Instance)
	m_Instance = new DeviceButtonManager();
    return *m_Instance;
}

  
DeviceButtonManager::DeviceButtonManager() : QObject(0L)
{
  QCopChannel *channel = new QCopChannel( "QPE/System", this );
  connect( channel, SIGNAL(received(const QCString&, const QByteArray&)),
	   this, SLOT(received(const QCString&, const QByteArray&)) );
  loadButtons();
}

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
 Reassigns the pressed action for \a button_index.
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
    QCopEnvelope("QPE/System", "deviceButtonMappingChanged()");
}

/*!
 Reassigns the held action for \a button_index. 
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
    QCopEnvelope("QPE/System", "deviceButtonMappingChanged()");
}

void DeviceButtonManager::loadButtons(bool factory)
{
    m_Buttons = getCustomDeviceButtons();

    Config buttonFile("ButtonSettings");
    if (!buttonFile.isValid())
	return;
    QString groupBase("Button"); // No tr
    int i=0;
    for (QValueListIterator<DeviceButton> it = m_Buttons.begin(); it != m_Buttons.end(); ++it,++i) {
	QString groupName = groupBase + QString::number(i);
	buttonFile.setGroup(groupName);
	if ( factory ) {
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
	}
    }
}

void DeviceButtonManager::received(const QCString& message, const QByteArray&)
{
    if (message == "deviceButtonMappingChanged()")
	loadButtons();
}

void DeviceButtonManager::factoryResetButtons()
{
    loadButtons(TRUE);
}
