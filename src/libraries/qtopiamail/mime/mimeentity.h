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


#ifndef MIME_ENTITY_H
#define MIME_ENTITY_H

#include "pstring.h"

#include "mimeheader.h"
#include "mimebody.h"

class MimeEntity
{
public:
    friend class MimeBody;

    MimeEntity(MimeEntity *parent = 0);
    MimeEntity(const MimeEntity &);
    virtual ~MimeEntity();

    virtual void fromString(const PString &s) = 0;
    virtual PString toString() const = 0;

    MimeHeader* header() const {return _header;};
    MimeBody* body() const {return _body;};

    MimeEntity *parent() {return _parent;};
    void reparent(MimeEntity *p) {_parent = p;};

protected:
    virtual void setPreamble(const PString &) = 0;
    virtual void setEpilogue(const PString &) = 0;

    virtual void parse(const PString &s) = 0;

    MimeHeader *_header;
    MimeBody *_body;

private:
    MimeEntity *_parent;
};

#endif
