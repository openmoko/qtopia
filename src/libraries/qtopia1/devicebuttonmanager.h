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
#ifndef DEVICE_BUTTON_MANAGER_H
#define DEVICE_BUTTON_MANAGER_H

#include <qobject.h>
#include <qvaluelist.h>
#include <qstring.h>
#include "devicebutton.h"

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
