/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#ifndef EXAMPLEKBDHANDLER_H
#define EXAMPLEKBDHANDLER_H

#ifdef QT_QWS_EXAMPLE

#include <QObject>
#include <QWSKeyboardHandler>

class QSocketNotifier;
class ExampleKbdHandler : public QObject, public QWSKeyboardHandler {
    Q_OBJECT
public:
    ExampleKbdHandler();
    ~ExampleKbdHandler();

private:
    QSocketNotifier *m_notify;
    int  kbdFD;
    bool shift;

private Q_SLOTS:
    void readKbdData();
};

#endif // QT_QWS_EXAMPLE

#endif // EXAMPLEKBDHANDLER_H
