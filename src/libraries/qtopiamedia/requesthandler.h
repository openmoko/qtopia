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

#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <servicerequest.h>
#include <qtopiaglobal.h>

class QTOPIAMEDIA_EXPORT RequestHandler
{
public:
    RequestHandler( RequestHandler* successor = 0 );
    virtual ~RequestHandler() {};

    virtual void execute( ServiceRequest* request );

protected:
    RequestHandler* head() const { return m_head; }

private:
    void propagateHead( RequestHandler* handler );

    RequestHandler *m_successor;
    RequestHandler *m_head;
};

inline RequestHandler::RequestHandler( RequestHandler* successor )
    : m_successor( successor )
{
    propagateHead( this );
}

inline void RequestHandler::propagateHead( RequestHandler* handler )
{
   m_head = handler;

    if( m_successor ) {
        m_successor->propagateHead( handler );
    }
}

#endif // REQUESTHANDLER_H
