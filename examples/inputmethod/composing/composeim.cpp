/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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
#include "composeim.h"
#include <QDebug>

ComposeIM::ComposeIM()
    : QWSInputMethod(),
    upper(false), isActive(false)
{
}

ComposeIM::~ComposeIM()
{
}

bool ComposeIM::filter(int unicode, int keycode, int modifiers, 
        bool isPress, bool /* autoRepeat */)
{
    if (!isActive)
        return false;

    if ((keycode > Qt::Key_Z || keycode < Qt::Key_A) && keycode != Qt::Key_Shift) {
        reset();
        return false;
    }

    if (isPress) {
        if (keycode == Qt::Key_Shift) {
            lastText = upper ? lastText.toLower() : lastText.toUpper();
            upper = !upper;
            sendPreeditString(lastText, 0);
        } else {
            if (!lastText.isEmpty())
                sendCommitString(lastText);

            lastText = QChar(unicode);
            upper = (modifiers & Qt::ShiftModifier) == Qt::ShiftModifier;
            sendPreeditString(lastText, 0);
        }
    }
    return true;
}

void ComposeIM::reset()
{
    if (!lastText.isEmpty())
        sendCommitString(lastText);
    upper = false;
    lastText = QString::null;
}

void ComposeIM::setActive(bool b)
{
    if (b && b != isActive) {
        lastText.clear();
        upper = false;
    }
    isActive = b;
}
