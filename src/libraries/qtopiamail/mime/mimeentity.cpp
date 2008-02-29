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


#include <qcstring.h>
#include "mimeentity.h"

MimeEntity::MimeEntity(MimeEntity *parent)
    : _parent(parent)
{
    _header = new MimeHeader(this);
    _body = new MimeBody(this);
}

MimeEntity::MimeEntity(const MimeEntity &other)
{
    _header = new MimeHeader(*other._header);
    _body = new MimeBody(*other._body);

    _parent = 0;
    _header->reparent(this);
    _body->reparent(this);
}

MimeEntity::~MimeEntity()
{
    delete _header;
    delete _body;
}

