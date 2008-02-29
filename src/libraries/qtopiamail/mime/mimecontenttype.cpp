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


#include "mimecontenttype.h"
#include "mimetools.h"

static PString stripWhiteAndQuotes(const PString &in)
{
    PString out =in.stripWhiteSpace();

    if ( out[0] == '"' )
        out = out.mid(1, out.length() - 2);

    return out;
}

MimeContentType::MimeContentType()
    : MimeHeaderField()
{
    _contentType = "text/plain";
}

MimeContentType::MimeContentType(const PString &s)
    : MimeHeaderField(s)
{
    _contentType = "text/plain";

    fromString(s);
}

MimeContentType::~MimeContentType()
{
}

void MimeContentType::parse(const PString &s)
{
    parseContentType( s );
}

// Simple for now.
PString MimeContentType::encode()
{
    PString str = _contentType;

    if ( !str.isEmpty() ) {
        if ( !_charset.isEmpty() )
            str += "; charset=" + _charset;
        if ( !_boundary.isEmpty() ) {
            str += "; boundary=\"" + _boundary + "\""; // No tr
        } else if ( !_name.isEmpty() ) {
            // Name does not make sense if a boundary is defined
            str += "; name=\"" + _name + "\""; // No tr
        }
    }

    return str;
}

PString MimeContentType::contentType() const
{
    return _contentType;
}

// Convenience function, assumes contenttype is correct format
PString MimeContentType::type() const
{
    int pos = _contentType.find("/");
    if ( pos == -1)
        return _contentType;

    return _contentType.mid(0, pos );
}

// Convenience function, assumes contenttype is correct format
PString MimeContentType::subType() const
{
    int pos = _contentType.find("/");
    if ( pos == -1)
        return _contentType;

    return _contentType.mid(pos+1);
}

PString MimeContentType::boundary() const
{
    return _boundary;
}

PString MimeContentType::charset() const
{
    if ( _charset.isEmpty() )
        return "us-ascii";

    return _charset;
}

PString MimeContentType::name() const
{
    return _name;
}

void MimeContentType::setContentType(const PString &s)
{
    _contentType = stripWhiteAndQuotes(s);
}

void MimeContentType::setBoundary(const PString &s)
{
    _boundary = stripWhiteAndQuotes(s);
}

void MimeContentType::setCharset(const PString &s)
{
    _charset = stripWhiteAndQuotes(s).lower();
    if ( _charset == "us-ascii" )
        _charset = "";
}

void MimeContentType::setName(const PString &s)
{
    _name = s;
}

//  Quick impl.  Need a better though.
void MimeContentType::createBoundary(uint seed)
{
    _boundary = _boundary.sprintf("-----4345=next_bound=%d----", seed);
}

void MimeContentType::parseContentType(const PString &s)
{
    const char *p = s.data();
    if ( !p ) return;

    // find content-type
    _contentType = "";  //need to reset default text/plain
    while ( *p != 0 ) {
        if ( *p == ' ' || *p == ';' ) {
            p++;
            break;
        }

        _contentType += *p;
        p++;
    }
    _contentType = _contentType.lower();

    //get any parameters
    PString str;
    while ( *p != 0 ) {
        str = "";
        while ( *p != 0 ) {
            if ( *p == ';' ) {
                p++;
                break;
            }

            str += *p;
            p++;
        }

        int pos;
        if ( (pos = str.find("boundary=", 0, false) ) > -1 ) { // No tr
            _boundary = stripWhiteAndQuotes( str.mid(pos + 9) );
        } else if ( (pos = str.find("charset=", 0, false) ) > -1 ) {
            _charset = stripWhiteAndQuotes( str.mid(pos + 8) ).lower();
            if ( _charset == "us-ascii" )
                _charset = "";
        } else if ( (pos = str.find("name=", 0, false) ) > -1 ) { // No tr
            _name = stripWhiteAndQuotes( str.mid(pos + 5).lower() );
        }
    }
}

