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
