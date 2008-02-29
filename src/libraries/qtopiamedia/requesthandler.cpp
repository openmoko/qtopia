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

#include "requesthandler.h"

#include <qtopialog.h>

#include <QtCore>

void RequestHandler::execute( ServiceRequest* request )
{
    // If successor exists, pass request to successor
    // Otherwise, use generic handler
    if( m_successor ) {
        m_successor->execute( request );
    } else {
        switch( request->type() )
        {
        case ServiceRequest::TriggerSlot:
            {
            TriggerSlotRequest *req = (TriggerSlotRequest*)request;
            QMetaObject::invokeMethod( req->receiver(), req->member() );
            }
            break;
        case ServiceRequest::Compound:
            {
            CompoundRequest* req = (CompoundRequest*)request;
            foreach( ServiceRequest* request, req->requests() ) {
                head()->execute( request->clone() );
            }
            break;
            }
        default:
            //REPORT_ERROR( ERR_UNSUPPORTED );
            qLog(Media) << "Service request type" << request->type();
            break;
        }

        delete request;
    }
}
