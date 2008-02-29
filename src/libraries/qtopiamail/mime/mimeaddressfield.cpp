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


#include "mimeaddressfield.h"

static PString unquoteString(const PString &str)
{
    PString tmp(str);

    if ( tmp[0] == '\"' && tmp[tmp.length() - 1] == '\"' )
        return tmp.mid(1, tmp.length() - 2);

    return tmp;
}

static PString quoteString(const PString &str)
{
    PString tmp(str);

    if ( tmp[0] != '\"' )
        tmp = "\"" + tmp;
    if (tmp[tmp.length() - 1] != '\"' )
        tmp += "\"";

    // deals with empty strings or a string containing only one double-quote
    if ( str.length() == 1 )
        return "";

    return tmp;
}

static void parseEmailAddress(const PString& fromStr, PString &nameStr, PString &emailStr)
{
    int pos;

    nameStr = fromStr;
    if ( (pos = fromStr.find("<", 0, false)) != -1) {
        nameStr = fromStr.mid(0, pos).stripWhiteSpace();

        nameStr = unquoteString(nameStr);

        pos++;
        int endPos = fromStr.find(">", pos, false);
        emailStr = fromStr.mid(pos, endPos - pos).stripWhiteSpace();
        if ( nameStr == "" )
            nameStr = emailStr;
    } else {
        emailStr = nameStr;
    }
}

static PString buildAddress(const PString &name, const PString &email)
{
    PString str = quoteString( name.stripWhiteSpace() );
    PString tmp = email.stripWhiteSpace();

    if ( tmp[0] != '<' )
        tmp = "<" + tmp;
    if (tmp[tmp.length() - 1] != '>' )
        tmp += ">";

    PString out;
    if ( str != "\"\"" )
        out = str;
    if ( tmp != "<>" ) {
        if ( out.isEmpty() )
            out = tmp;
        else
            out += " " + tmp;
    }

    return out;
}

MimeAddressField::MimeAddressField()
    : MimeHeaderField()
{
    _dirty = false;
}

MimeAddressField::MimeAddressField(const PString &s)
    : MimeHeaderField()
{
    fromString(s);
    _dirty = false;
}

MimeAddressField::~MimeAddressField()
{
}

PString MimeAddressField::address() const
{
    return _address;
}

PString MimeAddressField::name() const
{
    return _name;
}

PString MimeAddressField::email() const
{
    return _email;
}

void MimeAddressField::setName(const PString &str)
{
    _name = str;
    _dirty = true;
}

void MimeAddressField::setEmail(const PString &str)
{
    _email = str;
    _dirty = true;
}

void MimeAddressField::parse(const PString &s)
{
    _address = s;
    parseEmailAddress(_address, _name, _email);
}

PString MimeAddressField::encode()
{
    if ( !_dirty )
        return _address;

    _address  = buildAddress(_name, _email);
    _dirty = false;
    return _address;
}

