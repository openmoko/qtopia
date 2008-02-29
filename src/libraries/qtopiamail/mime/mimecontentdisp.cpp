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


#include "mimecontentdisp.h"
#include "mimetools.h"

static PString stripWhiteAndQuotes(const PString &in)
{
    PString out =in.stripWhiteSpace();

    if ( out[0] == '"' )
        out = out.mid(1, out.length() - 2);

    return out;
}

MimeContentDisposition::MimeContentDisposition()
    : MimeHeaderField()
{
    _contentDisposition = "attachment"; // No tr
}

MimeContentDisposition::MimeContentDisposition(const PString &s)
    : MimeHeaderField(s)
{
    _contentDisposition = "attachment"; // No tr

    fromString(s);
}

MimeContentDisposition::~MimeContentDisposition()
{
}

void MimeContentDisposition::parse(const PString &s)
{
    parseContentDisposition( s );
}

// Simple for now.
PString MimeContentDisposition::encode()
{
    PString str = _contentDisposition;

    if ( !str.isEmpty() ) {
        if ( !_filename.isEmpty() ) {
            str += "; filename=\"" + _filename + "\""; // No tr
        }
    }

    return str;
}

PString MimeContentDisposition::contentDisposition() const
{
    return _contentDisposition;
}

PString MimeContentDisposition::filename() const
{
    return _filename;
}

void MimeContentDisposition::setContentDisposition(const PString &s)
{
    _contentDisposition = stripWhiteAndQuotes(s);
}

void MimeContentDisposition::setFilename(const PString &s)
{
    _filename = s;
}

void MimeContentDisposition::parseContentDisposition(const PString &s)
{
    const char *p = s.data();
    if ( !p ) return;

    // find content-type
    _contentDisposition = "";
    while ( *p != 0 ) {
        if ( *p == ' ' || *p == ';' ) {
            p++;
            break;
        }

        _contentDisposition += *p;
        p++;
    }
    _contentDisposition = _contentDisposition.lower();

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
        if ( (pos = str.find("filename=", 0, false) ) > -1 ) { // No tr
            _filename = stripWhiteAndQuotes( str.mid(pos + 9) );
        }
    }
}

