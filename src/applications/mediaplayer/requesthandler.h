/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include "servicerequest.h"
#include <qtopiaglobal.h>

class RequestHandler
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

#endif // REQUESTHANDLER_H
