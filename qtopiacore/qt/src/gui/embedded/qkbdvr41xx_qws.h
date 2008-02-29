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
