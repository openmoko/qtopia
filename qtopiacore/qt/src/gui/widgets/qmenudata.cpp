/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "qmenudata.h"

#ifdef QT3_SUPPORT
#include <qaction.h>
#include <private/qaction_p.h>

/*!
    \class QMenuItem
    \brief The QMenuItem class represents an item in a menu.

    \compat

    Use QAction instead.
*/

/*!
    \compat
    Constructs a new menu item.
*/
QMenuItem::QMenuItem() : QAction((QWidget*)0)
{
}

void QMenuItem::setId(int id)
{
    d_func()->param = d_func()->id = id;
}

/*!
    \compat
    Returns the menu item's ID.
*/
int QMenuItem::id() const
{
    return d_func()->id;
}

void QMenuItem::setSignalValue(int param)
{
    d_func()->param = param;
}

/*!
    \compat
    Returns the signal value for the menu item.
*/
int QMenuItem::signalValue() const
{
    return d_func()->param;
}
#endif
