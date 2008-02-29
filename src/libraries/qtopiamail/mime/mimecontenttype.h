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


#ifndef MIME_CONTENTTYPE_H
#define MIME_CONTENTTYPE_H

#include "pstring.h"
#include "mimeheaderfield.h"

class MimeContentType : public MimeHeaderField
{
public:
    MimeContentType();
    MimeContentType(const PString &s);
    ~MimeContentType();

    PString contentType() const;
    PString type() const;
    PString subType() const;
    PString boundary() const;
    PString charset() const;
    PString name() const;

    void setContentType(const PString &);
    void setBoundary(const PString &);
    void setCharset(const PString &);
    void setName(const PString &);

    void createBoundary(uint seed);

protected:
    virtual void parse(const PString &);
    virtual PString encode();

    void parseContentType(const PString &s);


private:
    PString _contentType;
    PString _boundary, _charset, _name;
};

#endif
