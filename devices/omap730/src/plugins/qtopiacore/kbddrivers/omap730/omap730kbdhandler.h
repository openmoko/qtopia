/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef OMAP730KBDHANDLER_H
#define OMAP730KBDHANDLER_H

#ifdef QT_QWS_OMAP730

#include <QObject>
#include <QWSKeyboardHandler>

class QSocketNotifier;
class Omap730KbdHandler : public QObject, public QWSKeyboardHandler
{
    Q_OBJECT
public:
    Omap730KbdHandler();
    ~Omap730KbdHandler();

private:
    QSocketNotifier *m_notify;
    int  kbdFD;
    bool shift;
    unsigned int prev_key,prev_unicode;

private Q_SLOTS:
    void readKbdData();
};

#endif // QT_QWS_OMAP730

#endif // OMAP730KBDHANDLER_H
