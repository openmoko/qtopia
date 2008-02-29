/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef _KEYCLICK_H_
#define _KEYCLICK_H_

#include "qtopiainputevents.h"
#include <QObject>

class KeyClick : public QObject, public QtopiaKeyboardFilter
{
Q_OBJECT
public:
    KeyClick();
    virtual ~KeyClick();

protected:
    virtual bool filter(int unicode, int keycode, int modifiers,
                        bool press, bool autoRepeat);
    virtual void keyClick(int unicode, int keycode, int modifiers,
                          bool press, bool repeat) = 0;

private slots:
    void rereadVolume();

private:
    bool m_clickenabled;
};

#endif // _KEYCLICK_H_
