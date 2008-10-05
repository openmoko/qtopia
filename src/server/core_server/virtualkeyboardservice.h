/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
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
    void keyPress( QString keySequence );
    void sendKeyEvent( int unicode, int keycode, int modifiers,
                       bool isPress, bool autoRepeat );
    void processKeyEvent( int unicode, int keycode, int modifiers,
                          bool isPress, bool autoRepeat );
};

#endif
