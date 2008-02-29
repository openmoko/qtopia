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


#include "mimemessage.h"
#include "mimebodypart.h"

MimeMessage::MimeMessage()
    : MimeEntity(this)
{
}

MimeMessage::MimeMessage(const PString &s)
    : MimeEntity(this)
{
    fromString(s);
}

MimeMessage::MimeMessage(const MimeMessage &other)
    : MimeEntity(other)
{
    reparent(this);
    _preamble = other._preamble;
    _epilogue = other._epilogue;
}

MimeMessage::~MimeMessage()
{
}

void MimeMessage::fromString(const PString &s)
{
    parse(s);
}

PString MimeMessage::toString() const
{
    PString str = _header->toString();
    str += "\n\n";
    PString boundary = header()->contentType().boundary();

    if ( boundary != "" ) {
        str += _preamble;
        str += "\n--" + boundary + "\n";
    }

    str += _body->toString();

    // we know there is no header fields in the epilogue, so we add
    // an extra \n
    if ( boundary != "" ) {
        str += "\n--" + boundary + "--\n";
        str += _epilogue;
    }

    return str;
}

// 95% of mails are simle text only bodies.  If this one returns true, you only
// need to call _body->text() to get the contents
bool MimeMessage::textOnly() const
{
    return _body->textOnly();
}

// Returns a single string consisting of all body-text in the message.
PString MimeMessage::bodyText(bool html) const
{
    if ( textOnly() )
        return _body->text();

    PString txt;
    MimeBodyPart *bp;
    for ( uint u = 0; u < _body->count(); u++ ) {
        bp = _body->multiPartAt( u );
        MimeContentType ct = bp->header()->contentType();
        PString type = ct.type();

        if ( type == "text") { // No tr
            PString subtype = ct.subType();

            if ( !html && subtype == "plain" ) { // No tr
                if ( ct.name().isEmpty() && bp->header()->contentDisposition().filename().isEmpty() )
                    txt += bp->body()->text();
            } else if ( html && subtype != "plain" ) { // No tr
                //!plain means rich, html, etc..
                if ( ct.name().isEmpty() && bp->header()->contentDisposition().filename().isEmpty() )
                    txt += bp->body()->text();
            }
        }
    }

    return txt;
}

void MimeMessage::clear()
{
    _header->clear();
    _body->clear();
    _preamble = "";
    _epilogue = "";
}

void MimeMessage::setPreamble(const PString &str)
{
    _preamble = str;
}

void MimeMessage::setEpilogue(const PString &str)
{
    _epilogue = str;
}

void MimeMessage::parse(const PString &s)
{
    int pos = s.find("\n\n", 0);
    if ( pos == -1 ) {
        _header->fromString( s );
        return;
    }

    _header->fromString( s.mid(0, pos) );
    pos += 2;   //because of \n\n

    PString boundary = "--" + header()->contentType().boundary();
    int start = pos;
    int end = s.length();
    if ( boundary != "--" ) {
        start =  s.find(boundary, start );

        if ( start > -1 ) {
            _preamble = s.mid(pos, start - pos);
            start = s.find("\n", ++start); //get beginning of body
            start++;
        } else {
            start = pos;
            qWarning("hmm, could not find beginning boundary");
        }

        // We might be a sub message so we need to find
        // any end boundaries if they exist
        if ( (end = s.find(boundary + "--", start) ) == -1) {
            end = s.length();
        } else {
            int epStart = end + boundary.length() + 3; //+3 : "--\n"
            if ( epStart < (int) s.length() )
                _epilogue = s.mid( epStart, s.length() - epStart);
        }
    }

    _body->fromString( s.mid(start, end - start) );
}

