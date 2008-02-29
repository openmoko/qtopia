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

#ifndef QKBDVR41XX_QWS_H
#define QKBDVR41XX_QWS_H

#include <QtGui/qkbd_qws.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#if !defined(QT_NO_QWS_KEYBOARD) && !defined(QT_NO_QWS_KBD_VR41XX)

class QWSVr41xxKbPrivate;

class QWSVr41xxKeyboardHandler : public QWSKeyboardHandler
{
public:
    explicit QWSVr41xxKeyboardHandler(const QString&);
    virtual ~QWSVr41xxKeyboardHandler();

private:
    QWSVr41xxKbPrivate *d;
};

#endif // QT_NO_QWS_KBD_VR41XX

QT_END_HEADER

#endif // QKBDVR41XX_QWS_H
