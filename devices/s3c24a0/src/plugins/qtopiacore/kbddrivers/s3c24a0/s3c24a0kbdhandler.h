/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef S3C24A0KBDHANDLER_H
#define S3C24A0KBDHANDLER_H

#ifdef QT_QWS_S3C24A0

#include <QObject>
#include <QWSKeyboardHandler>

class QSocketNotifier;
class S3c24a0KbdHandler : public QObject, public QWSKeyboardHandler
{
    Q_OBJECT
public:
    S3c24a0KbdHandler();
    ~S3c24a0KbdHandler();

private:
    QSocketNotifier *m_notify;
    int  kbdFD;
    bool shift;

private Q_SLOTS:
    void readKbdData();
};

#endif // QT_QWS_S3C24A0

#endif // S3C24A0KBDHANDLER_H
