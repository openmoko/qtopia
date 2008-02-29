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

#include "mimebodypart.h"

//  A mime body part must always have a parent of type entity
MimeBodyPart::MimeBodyPart(MimeEntity *parent, const PString &s)
    : MimeEntity(parent)
{
    parse(s);
}

//  Constructor for outgoing mime messages (E.g. user created)
MimeBodyPart::MimeBodyPart(MimeEntity *parent)
    : MimeEntity(parent)
{
}

MimeBodyPart::MimeBodyPart(const MimeBodyPart &other)
    : MimeEntity(other)
{
    reparent(this);
    _preamble = other._preamble;
    _epilogue = other._epilogue;
}

MimeBodyPart::~MimeBodyPart()
{
}

void MimeBodyPart::fromString(const PString &s)
{
    parse(s);
}

PString MimeBodyPart::toString() const
{
    PString str = _header->toString();

    // special case where no header fields are defined
    if ( str.length() == 0 ) {
        str += "\n";
    } else {
        str += "\n\n";
    }

    PString boundary = "--" + header()->contentType().boundary();
    if ( boundary != "--" ) {
        str += _preamble;
        str += boundary + "\n";
    }

    str += _body->toString();

    if ( boundary != "--" ) {
        str += boundary + "--\n";
        str += _epilogue;
    }

    return str;
}

void MimeBodyPart::parse(const PString &s)
{
    parseBodyPart( s );
}

void MimeBodyPart::parseBodyPart(const PString &s)
{
    int pos;

    // Special case, the boundary always ends with a \n, so if
    // the first char is \n the pos must be 1.
    if ( s[0] == '\n' ) {
        pos = 0;
    } else
        pos = s.find("\n\n", 0 );

    if ( pos == -1 ) {
        qWarning("MimeBodyPart::parseBodyPart: cannot distinguish body part header, quitting");
        return;
    }

    _header->fromString( s.mid(0, pos) );

    pos += 2;   //because of \n
    if (pos == 2 )
        pos--;      //because of the special case

    PString boundary = "--" + header()->contentType().boundary();

    int start = pos;
    int end = s.length();

    // new boundary defined.  this happens if the body part
    // is itself a new multipart type.  Although this is almost
    // the same as mimemessage, there are slight differences with
    // catching the boundary
    if ( boundary != "--") {
        start =  s.find(boundary, start );
        if ( start > -1 ) {
            _preamble = s.mid(pos, start - pos);

            start = s.find("\n", ++start); //get beginning of body
            start++;
        } else {
            start = pos;
            qWarning("MimeBodyPart::parseBodyPart: could not find beginning boundary");
        }

        // We might be a sub message so we need to find
        // any end boundaries if they exist
        if ( (end = s.find(boundary + "--", start) ) == -1) {
            end = s.length();
        } else {
            uint epStart = end + boundary.length() + 3; //+3 : "--\n"
            if ( epStart < s.length() )
                _epilogue = s.mid( epStart, s.length() - epStart);
        }
    }

    _body->fromString( s.mid(start, end - start) );
}

