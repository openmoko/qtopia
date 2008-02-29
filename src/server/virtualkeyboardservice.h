/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef VIRTUALKEYBOARDSERVICE_H
#define VIRTUALKEYBOARDSERVICE_H

#include <qtopiaabstractservice.h>

class VirtualKeyboard;
class QString;
class QByteArray;

class VirtualKeyboardService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    VirtualKeyboardService( QObject *parent = 0);
    ~VirtualKeyboardService();

public slots:
    void keyPress( int key );
    void sendKeyEvent( int unicode, int keycode, int modifiers,
                       bool isPress, bool autoRepeat );
    void processKeyEvent( int unicode, int keycode, int modifiers,
                          bool isPress, bool autoRepeat );
};

#endif
