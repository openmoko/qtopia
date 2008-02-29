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
#ifndef DEVICE_BUTTON_H
#define DEVICE_BUTTON_H

#include <qpixmap.h>
#include <qstring.h>
#include <qtopia/services.h>

/**
 * This class represents a physical button on a Qtopia device.  A device may
 * have n "user programmable" buttons, which are number 1..n.  The location
 * and number of buttons will vary from device to device.  userText() and pixmap()
 * may be used to describe this button to the user in help documentation.
 */
class QTOPIA_EXPORT DeviceButton {
 public:
  DeviceButton();
  virtual ~DeviceButton();

  ushort keycode() const;
  QString userText() const;
  QPixmap pixmap() const;
  ServiceRequest pressedAction() const;
  ServiceRequest heldAction() const;

  void setKeycode(ushort keycode);
  void setUserText(const QString& text);
  void setPixmap(const QString& pmn);
  void setPressedAction(const ServiceRequest& qcopMessage);
  void setHeldAction(const ServiceRequest& qcopMessage);
  bool operator==(const DeviceButton &e) const;   
 
 private:
  ushort m_Keycode;
  QString m_UserText;
  QString m_PixmapName;
  QPixmap m_Pixmap;  
  ServiceRequest m_PressedAction;
  ServiceRequest m_HeldAction;
};

#define QTOPIA_DEFINED_DEVICEBUTTON
#include <qtopia/qtopiawinexport.h>
#endif
