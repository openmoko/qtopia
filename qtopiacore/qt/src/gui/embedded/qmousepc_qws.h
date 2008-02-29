/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMOUSEPC_QWS_H
#define QMOUSEPC_QWS_H

#include <QtGui/qmouse_qws.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_QWS_MOUSE_PC

class QWSPcMouseHandlerPrivate;

class QWSPcMouseHandler : public QWSMouseHandler
{
public:
    explicit QWSPcMouseHandler(const QString & = QString(),
                               const QString & = QString());
    ~QWSPcMouseHandler();

    void suspend();
    void resume();
protected:
    QWSPcMouseHandlerPrivate *d;
};

#endif // QT_NO_QWS_MOUSE_PC

QT_END_HEADER

#endif // QMOUSEPC_QWS_H
