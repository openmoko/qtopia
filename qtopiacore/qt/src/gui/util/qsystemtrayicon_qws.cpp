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

#include "qsystemtrayicon_p.h"

#ifndef QT_NO_SYSTEMTRAYICON

void QSystemTrayIconPrivate::install_sys()
{
}

void QSystemTrayIconPrivate::remove_sys()
{
}

QPoint QSystemTrayIconPrivate::globalPos_sys() const
{
    return QPoint();
}

void QSystemTrayIconPrivate::updateIcon_sys()
{
}

void QSystemTrayIconPrivate::updateMenu_sys()
{
}

void QSystemTrayIconPrivate::updateToolTip_sys()
{
}

bool QSystemTrayIconPrivate::isSystemTrayAvailable_sys()
{
    return false;
}

void QSystemTrayIconPrivate::showMessage_sys(const QString &message,
                                             const QString &title,
                                             QSystemTrayIcon::MessageIcon icon,
                                             int msecs)
{
    Q_UNUSED(message);
    Q_UNUSED(title);
    Q_UNUSED(icon);
    Q_UNUSED(msecs);
}

#endif // QT_NO_SYSTEMTRAYICON
