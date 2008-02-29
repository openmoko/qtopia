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

#ifndef QMOUSELINUXTP_QWS_H
#define QMOUSELINUXTP_QWS_H

#include <QtGui/qmouse_qws.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_QWS_MOUSE_LINUXTP

class QWSLinuxTPMouseHandlerPrivate;

class QWSLinuxTPMouseHandler : public QWSCalibratedMouseHandler
{
    friend class QWSLinuxTPMouseHandlerPrivate;
public:
    explicit QWSLinuxTPMouseHandler(const QString & = QString(),
                                    const QString & = QString());
    ~QWSLinuxTPMouseHandler();

    void suspend();
    void resume();
protected:
    QWSLinuxTPMouseHandlerPrivate *d;
};

#endif // QT_NO_QWS_MOUSE_LINUXTP

QT_END_HEADER

#endif // QMOUSELINUXTP_QWS_H
