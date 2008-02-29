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
#ifndef DEVICE_BUTTON_H
#define DEVICE_BUTTON_H

#include <qpixmap.h>
#include <qstring.h>
#include <qtopiaservices.h>

/**
 * This class represents a physical button on a Qtopia device.  A device may
 * have n "user programmable" buttons, which are number 1..n.  The location
 * and number of buttons will vary from device to device.  userText() and pixmap()
 * may be used to describe this button to the user in help documentation.
 */
class QTOPIA_EXPORT QDeviceButton {
public:
    QDeviceButton();
    virtual ~QDeviceButton();

    int keycode() const;
    QString context() const;
    QString userText() const;
    QPixmap pixmap() const;
    QtopiaServiceRequest pressedAction() const;
    QtopiaServiceRequest heldAction() const;
    bool pressedActionMappable() const { return m_PressedActionMapable; }
    bool heldActionMappable() const { return m_HeldActionMapable; }

    void setKeycode(int keycode);
    void setContext(const QString& context);
    void setUserText(const QString& text);
    void setPixmap(const QString& pmn);
    void setPressedAction(const QtopiaServiceRequest& qcopMessage);
    void setHeldAction(const QtopiaServiceRequest& qcopMessage);
    void setPressedActionMappable(bool);
    void setHeldActionMappable(bool);
    bool operator==(const QDeviceButton &e) const;

private:
    int m_Keycode;
    QString m_UserText;
    QString m_PixmapName;
    QPixmap m_Pixmap;
    QString m_Context;
    QtopiaServiceRequest m_PressedAction;
    QtopiaServiceRequest m_HeldAction;
    bool m_PressedActionMapable;
    bool m_HeldActionMapable;
};

#define QTOPIA_DEFINED_DEVICEBUTTON
#endif
