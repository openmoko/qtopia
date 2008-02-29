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


#include "mimeheaderfield.h"
#include "mimetools.h"

/*  Empty charset defaults to US-ASCII
    This class will handle folding of header fields
*/
MimeHeaderField::MimeHeaderField()
{
}

MimeHeaderField::MimeHeaderField(const PString &s)
{
    fromString(s);
}

MimeHeaderField::~MimeHeaderField()
{
}

void MimeHeaderField::fromString(const PString &s)
{
    parse ( decodeLocale(s) );
}

PString MimeHeaderField::toString()
{
    //call encodeLocale (need to write it first though...)

    return encodeLocale( encode() );
}

const PString& MimeHeaderField::contents()
{
    return _str;
}

void MimeHeaderField::setCharset(const PString &s)
{
    _charset = s;
}

//  Dummy, must be reimplemented
void MimeHeaderField::parse(const PString &)
{
}

PString MimeHeaderField::encode()
{
    return _str;
}

PString MimeHeaderField::decodeLocale(const PString &s)
{
    //need to store the str
    _str = MimeTools::decodeLocale(s.simplifyWhiteSpace(), _charset);
    return _str;
}

PString MimeHeaderField::encodeLocale(const PString &s)
{
    //we only store the decoded form
    return MimeTools::encodeLocale(s, _charset);
}

