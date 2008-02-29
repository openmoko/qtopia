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
#ifndef DEVICE_BUTTON_MANAGER_H
#define DEVICE_BUTTON_MANAGER_H

#include <qobject.h>
#include <qvaluelist.h>
#include <qstring.h>
#include <qtopia/devicebutton.h>

class ServiceRequest;

class QTOPIA_EXPORT DeviceButtonManager : QObject {
  Q_OBJECT

 public:
  static DeviceButtonManager& instance();
  
  /**
   * Returns the available buttons on this device.  The number and location
   * of buttons will vary depending on the device.  Button numbers will be assigned
   * by the device manufacturer and will be from most preferred button to least preffered
   * button.  Note that this list only contains "user mappable" buttons.
   */
  const QValueList<DeviceButton>& buttons() const;

  /**
   * Returns the DeviceButton for the \a keyCode.  If \a keyCode is not found, it
   * returns 0L
   */
  const DeviceButton* buttonForKeycode(ushort keyCode);

  void remapPressedAction(int button, const ServiceRequest& qcopMessage);
  void remapHeldAction(int button, const ServiceRequest& qcopMessage);

  void factoryResetButtons();

 protected:
  DeviceButtonManager();
  virtual ~DeviceButtonManager();

  private slots:
    void received(const QCString& message, const QByteArray& data);

 private:
  void loadButtons(bool factory=FALSE);

  static DeviceButtonManager* m_Instance;
  QValueList<DeviceButton> m_Buttons;
};

#define QTOPIA_DEFINED_DEVICEBUTTONMANAGER
#include <qtopia/qtopiawinexport.h>

#endif
