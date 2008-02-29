/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
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
