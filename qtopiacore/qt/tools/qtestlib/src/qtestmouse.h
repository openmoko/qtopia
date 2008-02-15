/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTESTMOUSE_H
#define QTESTMOUSE_H

#if 0
// inform syncqt
#pragma qt_no_master_include
#endif

#include <QtTest/qtest_global.h>
#include <QtTest/qtestassert.h>
#include <QtTest/qtestsystem.h>
#include <QtTest/qtestspontaneevent.h>

#include <QtCore/qpoint.h>
#include <QtCore/qstring.h>
#include <QtGui/qapplication.h>
#include <QtGui/qevent.h>
#include <QtGui/qwidget.h>

QT_BEGIN_HEADER

namespace QTest
{
    enum MouseAction { MousePress, MouseRelease, MouseClick, MouseDClick, MouseMove };

    static void mouseEvent(MouseAction action, QWidget *widget, Qt::MouseButton button,
                           Qt::KeyboardModifiers stateKey, QPoint pos, int delay=-1)
    {
        QTEST_ASSERT(widget);
        extern int Q_TESTLIB_EXPORT defaultMouseDelay();

        if (delay == -1 || delay < defaultMouseDelay())
            delay = defaultMouseDelay();
        if(delay > 0)
            QTest::qWait(delay);

        if (pos.isNull())
            pos = widget->rect().center();

        if (action == MouseClick) {
            mouseEvent(MousePress, widget, button, stateKey, pos);
            mouseEvent(MouseRelease, widget, button, stateKey, pos);
            return;
        }

        QTEST_ASSERT(button == Qt::NoButton || button & Qt::MouseButtonMask);
        QTEST_ASSERT(stateKey == 0 || stateKey & Qt::KeyboardModifierMask);

        stateKey &= Qt::KeyboardModifierMask;

        QMouseEvent me(QEvent::User, QPoint(), Qt::LeftButton, button, stateKey);
        switch (action)
        {
            case MousePress:
                me = QMouseEvent(QEvent::MouseButtonPress, pos, widget->mapToGlobal(pos), button, button, stateKey);
                break;
            case MouseRelease:
                me = QMouseEvent(QEvent::MouseButtonRelease, pos, widget->mapToGlobal(pos), button, 0, stateKey);
                break;
            case MouseDClick:
                me = QMouseEvent(QEvent::MouseButtonDblClick, pos, widget->mapToGlobal(pos), button, button, stateKey);
                break;
            case MouseMove:
                QCursor::setPos(widget->mapToGlobal(pos));
                qApp->processEvents();
                return;
            default:
                QTEST_ASSERT(false);
        }
        reinterpret_cast<QSpontaneKeyEvent *>(&me)->setSpontaneous();
        if (!qApp->notify(widget, &me))
            QTest::qWarn("Mouse event not accepted by receiving widget");

    }

    inline void mousePress(QWidget *widget, Qt::MouseButton button, Qt::KeyboardModifiers stateKey = 0,
                           QPoint pos = QPoint(), int delay=-1)
    { mouseEvent(MousePress, widget, button, stateKey, pos, delay); }
    inline void mouseRelease(QWidget *widget, Qt::MouseButton button, Qt::KeyboardModifiers stateKey = 0,
                             QPoint pos = QPoint(), int delay=-1)
    { mouseEvent(MouseRelease, widget, button, stateKey, pos, delay); }
    inline void mouseClick(QWidget *widget, Qt::MouseButton button, Qt::KeyboardModifiers stateKey = 0,
                           QPoint pos = QPoint(), int delay=-1)
    { mouseEvent(MouseClick, widget, button, stateKey, pos, delay); }
    inline void mouseDClick(QWidget *widget, Qt::MouseButton button, Qt::KeyboardModifiers stateKey = 0,
                            QPoint pos = QPoint(), int delay=-1)
    { mouseEvent(MouseDClick, widget, button, stateKey, pos, delay); }
    inline void mouseMove(QWidget *widget, QPoint pos = QPoint(), int delay=-1)
    { mouseEvent(MouseMove, widget, Qt::NoButton, 0, pos, delay); }

}

QT_END_HEADER

#endif // QTESTMOUSE_H
