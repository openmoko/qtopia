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

#ifndef __QOBEXNAMESPACE_H__
#define __QOBEXNAMESPACE_H__


namespace QObex {

    enum Error {
        NoError,
        LinkError,
        TransportConnectionError,
        RequestFailed,
        UnknownError = 100
    };

    enum State {
        Ready,
        Connecting,
        Disconnecting,
        Streaming,
        Closed = 100
    };

};

#endif
