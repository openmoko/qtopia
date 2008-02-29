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


#ifndef MIME_ADDRESS_FIELD_H
#define MIME_ADDRESS_FIELD_H

#include "pstring.h"
#include "mimeheaderfield.h"

class MimeAddressField : public MimeHeaderField
{
public:
    MimeAddressField();
    MimeAddressField(const PString &s);
    ~MimeAddressField();

    PString address() const;
    PString name() const;
    PString email() const;

    void setName(const PString &);
    void setEmail(const PString &);

protected:
    virtual void parse(const PString &);
    virtual PString encode();

private:
    PString _address;
    PString _email;
    PString _name;
    bool _dirty;
};

#endif
