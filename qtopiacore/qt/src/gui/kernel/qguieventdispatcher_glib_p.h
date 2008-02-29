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

#ifndef QGUIEVENTDISPATCHER_GLIB_P_H
#define QGUIEVENTDISPATCHER_GLIB_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/private/qeventdispatcher_glib_p.h>

class QGuiEventDispatcherGlibPrivate;

class QGuiEventDispatcherGlib : public QEventDispatcherGlib
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGuiEventDispatcherGlib)

public:
    explicit QGuiEventDispatcherGlib(QObject *parent = 0);
    ~QGuiEventDispatcherGlib();

    bool processEvents(QEventLoop::ProcessEventsFlags flags);

    void startingUp();
};

#endif // QGUIEVENTDISPATCHER_GLIB_P_H
