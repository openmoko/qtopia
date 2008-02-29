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


#ifndef MIME_HEADER_FIELD_H
#define MIME_HEADER_FIELD_H

#include "pstring.h"

class MimeHeaderField
{
public:
    MimeHeaderField();
    MimeHeaderField(const PString &s);
    virtual ~MimeHeaderField();

    void fromString(const PString &s);
    PString toString();

    // differs from toString in the sense that it don't add
    // charset encodings or fold long lines
    const PString& contents();

    PString charset() const;
    void setCharset(const PString &s);

protected:
    virtual void parse(const PString &);
    virtual PString encode();

    PString decodeLocale(const PString &s);
    PString encodeLocale(const PString &s);

private:
    PString _str;
    PString _charset;
};

/* Inline methods   */

inline PString MimeHeaderField::charset() const
{
    return _charset;
}

#endif
