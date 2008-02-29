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

#ifndef QEVENTDISPATCHER_X11_P_H
#define QEVENTDISPATCHER_X11_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "private/qeventdispatcher_unix_p.h"

class QEventDispatcherX11Private;

class QEventDispatcherX11 : public QEventDispatcherUNIX
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QEventDispatcherX11)

public:
    explicit QEventDispatcherX11(QObject *parent = 0);
    ~QEventDispatcherX11();

    bool processEvents(QEventLoop::ProcessEventsFlags flags);
    bool hasPendingEvents();

    void flush();

    void startingUp();
    void closingDown();

protected:
    int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
               timeval *timeout);
};

#endif // QEVENTDISPATCHER_X11_P_H
