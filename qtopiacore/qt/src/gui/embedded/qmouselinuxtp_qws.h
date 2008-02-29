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
