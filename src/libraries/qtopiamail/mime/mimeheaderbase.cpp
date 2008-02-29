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


#include "mimeheaderbase.h"

MimeHeaderBase::MimeHeaderBase()
{
}

MimeHeaderBase::MimeHeaderBase(const PString &s)
{
    fromString(s);
}

MimeHeaderBase::~MimeHeaderBase()
{
}

void MimeHeaderBase::fromString(const PString &s)
{
    parse(s);
}

PString MimeHeaderBase::toString() const
{
    PString str;
    QValueList<PString> list;
    QMap<PString,PString> f = fields;
    QMap<PString,PString>::Iterator it;

    if ( ( ( it = f.find("message-id") ) != f.end() ) )  {
        list.append( "Message-Id: " + *it );
        f.remove( it );
    }
    if ( ( ( it = f.find("in-reply-to") ) != f.end() ) )  {
        list.append( "In-Reply-To: " + *it );
        f.remove( it );
    }

    if ( ( ( it = f.find("from") ) != f.end() ) )  {
        list.append( "From: " + *it );
        f.remove( it );
    }
    if ( ( ( it = f.find("reply-to") ) != f.end() ) )  {
        list.append( "Reply-To: " + *it );
        f.remove( it );
    }
    if ( ( ( it = f.find("to") ) != f.end() ) )  {
        list.append( "To: " + *it );
        f.remove( it );
    }
    if ( ( ( it = f.find("cc") ) != f.end() ) )  {
        list.append( "CC: " + *it );
        f.remove( it );
    }
    if ( ( ( it = f.find("bcc") ) != f.end() ) )  {
        list.append( "BCC: " + *it );
        f.remove( it );
    }
    if ( ( ( it = f.find("date") ) != f.end() ) )  {
        list.append( "Date: " + *it );
        f.remove( it );
    }
    if ( ( ( it = f.find("subject") ) != f.end() ) )  {
        list.append( "Subject: " + *it );
        f.remove( it );
    }

    // Mime defined fields
    if ( ( ( it = f.find("mime-version") ) != f.end() ) )  {
        list.append( "Mime-Version: " + *it );
        f.remove( it );
    }
    if ( ( ( it = f.find("content-type") ) != f.end() ) )  {
        list.append( "Content-Type: " + *it );
        f.remove( it );
    }
    if ( ( ( it = f.find("content-transfer-encoding") ) != f.end() ) )  {
        list.append( "Content-Transfer-Encoding: " + *it );
        f.remove( it );
    }
    if ( ( ( it = f.find("content-disposition") ) != f.end() ) )  {
        list.append( "Content-Disposition: " + *it );
        f.remove( it );
    }

    //  Add remaining unknown headers to top
    for ( it = f.begin(); it != f.end(); ++it ) {
        PString str = it.key() + ": ";
        str += *it;
        list.prepend( str);
    }

    //we should handle folding here
    for (QValueList<PString>::Iterator e = list.begin(); e != list.end(); ++e ) {
        str += *e + "\n";
    }

    // For semantic reasons the last header does not have a \n
    // attached. (the entity parsing relies on this fact)
    return str.stripWhiteSpace();
}

void MimeHeaderBase::setField(const PString &f, const PString &b)
{
    fields[f] = b;
}

void MimeHeaderBase::clear()
{
    fields.clear();
}

void MimeHeaderBase::parse(const PString &s)
{
    parseHeaderBase( s );
}

/*  TODO: Optimize this one */
void MimeHeaderBase::parseHeaderBase(const PString &s)
{
    int pos = 0, lineEnd = 0, endPos = 0;
    bool continued;

    int len = s.length();
    while (endPos < len ) {

        do {
            continued = false;
            if ( ( endPos = s.find("\n", lineEnd) ) == -1) {
                endPos = len;
            }
            lineEnd = endPos + 1;

            if ( endPos < len ) {
                if ( s[lineEnd] == ' ' || s[lineEnd] == '\t') {
                    continued = true;
                }
            }
        } while( continued );

        /*  Above should be ok, but these are way to slow
            (only here to aid the design speed) */
        PString field( s.mid(pos, lineEnd - pos) );
        uint sep = field.find(":");

        PString header = field.mid(0, sep ).lower();
        PString body = field.mid( sep+1 ).simplifyWhiteSpace();
//
//      if ( body[body.length() - 1] != '\n')
//          body += '\n';

        fields.insert( header, body );

        pos = lineEnd;
    }
}

