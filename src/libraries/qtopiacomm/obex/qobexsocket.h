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

#ifndef __QOBEXSOCKET_H__
#define __QOBEXSOCKET_H__

#include <qtopiaglobal.h>

class QTOPIACOMM_EXPORT QObexSocket
{
public:
    QObexSocket();

    virtual ~QObexSocket();

    virtual bool connect() = 0;
    virtual bool close();

    void setHandle(void *handle) { m_handle = handle; }
    void * handle() { return m_handle; }

    bool isConnected() const;

protected:
    void * m_handle;

private:
    Q_DISABLE_COPY(QObexSocket)
};

#endif
