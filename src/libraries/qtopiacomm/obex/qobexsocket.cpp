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

#include <qobexsocket.h>

#include <openobex/obex.h>

QObexSocket::QObexSocket()
{
    m_handle = 0;
}

QObexSocket::~QObexSocket()
{
    OBEX_Cleanup(static_cast<obex_t *>(m_handle));
    m_handle = NULL;
}

bool QObexSocket::connect()
{
    return false;
}

bool QObexSocket::close()
{
    int ret = OBEX_TransportDisconnect(static_cast<obex_t *>(m_handle));
    m_handle = NULL;

    if (ret == -1)
        return false;

    return true;
}

bool QObexSocket::isConnected() const
{
    return m_handle != NULL;
}
