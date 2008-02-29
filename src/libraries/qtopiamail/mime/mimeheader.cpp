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


#include "mimeheader.h"
#include "mimeentity.h"

using namespace Mime;

TransferEncoding MimeHeader::transferEncodingId( const PString &str )
{
    TransferEncoding te = EightBit;
    if (str.lower() == "base64") // No tr
        te = Base64;
    else if ( str.lower() == "quoted-printable")  // No tr
        te = QuotedPrintable;
    return te;
}

MimeHeader::MimeHeader(MimeEntity *parent)
    : _parent(parent)
{
    _transferEncoding = EightBit;
}

MimeHeader::MimeHeader(MimeEntity *parent, const PString &s)
    : _parent(parent)
{
    _transferEncoding = EightBit;

    fromString(s);
}

MimeHeader::~MimeHeader()
{
}

void MimeHeader::parse(const PString &s)
{
    MimeHeaderBase::parse( s );

    _transferEncoding = transferEncodingId( field("content-transfer-encoding").stripWhiteSpace() );
}

MimeAddressField MimeHeader::from() const
{
    return MimeAddressField( field("from") ); // No tr
}

MimeAddressField MimeHeader::replyTo() const
{
    return MimeAddressField( field("reply-to") );
}

MimeAddressField MimeHeader::inReplyTo() const
{
    return MimeAddressField( field("in-reply-to") );
}

MimeHeaderField MimeHeader::organization() const
{
    return MimeHeaderField( field("organization") ); // No tr
}

MimeHeaderField MimeHeader::subject() const
{
    return MimeHeaderField( field("subject") ); // No tr
}

MimeHeaderField MimeHeader::messageId() const
{
    return MimeHeaderField( field("message-id") );
}

MimeDateField MimeHeader::date() const
{
    return  MimeDateField( field("date") ); // No tr
}

QValueList<MimeAddressField> MimeHeader::to() const
{
    PString s = field("to").simplifyWhiteSpace(); // No tr

    return asList( s );
}

QValueList<MimeAddressField> MimeHeader::cc() const
{
    PString s = field("cc").simplifyWhiteSpace();

    return asList( s );
}

QValueList<MimeAddressField> MimeHeader::bcc() const
{
    PString s = field("bcc").simplifyWhiteSpace();

    return asList( s );
}

void MimeHeader::setTo(QValueList<MimeAddressField> l)
{
    setField("to", fromList(l)); // No tr
}

void MimeHeader::setCc(QValueList<MimeAddressField> l)
{
    setField("cc", fromList(l));
}

void MimeHeader::setBcc(QValueList<MimeAddressField> l)
{
    setField("bcc", fromList(l) );
}

QValueList<MimeAddressField> MimeHeader::asList(const PString &s) const
{
    QValueList<MimeAddressField> f;

    if ( s == "" )
        return f;

    uint pos = 0;
    uint beg = 0;
    uint len = s.length();
    while ( pos < len ) {
        if ( s[pos] != ',' )
            pos++;
        else {
            f.append( MimeAddressField( s.mid(beg, pos - beg) ) );
            beg = ++pos;
        }

        if ( pos >= len && beg < pos) {
            PString str = s.mid(beg, pos - beg);
            f.append( MimeAddressField( str ) );
        }
    }

    return f;
}

PString MimeHeader::fromList(QValueList<MimeAddressField> &list) const
{
    PString str;
    QValueList<MimeAddressField>::Iterator it = list.begin();

    while ( it != list.end() ) {
        if ( str.isEmpty() )
            str = (*it).toString();
        else
            str += ", " + (*it).toString();

        ++it;
    }

    return str;
}

void MimeHeader::setTransferEncoding(TransferEncoding te)
{
    _transferEncoding = te;
    PString str = "8bit"; // No tr

    if ( te == Base64 ) {
        str = "base64"; // No tr
    } else if ( te == QuotedPrintable ) {
        str = "quoted-printable";
    }

    setField("content-transfer-encoding", str );
}

TransferEncoding MimeHeader::transferEncoding() const
{
    return _transferEncoding;
}

MimeContentType MimeHeader::contentType() const
{
    return MimeContentType( field("content-type") );
}

MimeContentDisposition MimeHeader::contentDisposition() const
{
    return MimeContentDisposition( field("content-disposition") );
}

void MimeHeader::setFrom(MimeAddressField f)
{
    setField("from", f.toString() ); // No tr
}

// Convenience function
void MimeHeader::setFrom(const PString &str)
{
    MimeAddressField f( str );
    setField("from", f.toString() ); // No tr
}

void MimeHeader::setReplyTo(MimeAddressField &f)
{
    setField("reply-to", f.toString() );
}

void MimeHeader::setReplyTo(const PString &str)
{
    MimeAddressField f(str);
    setField("reply-to", f.toString() );
}

void MimeHeader::setInReplyTo(MimeAddressField &f)
{
    setField("in-reply-to", f.toString() );
}

void MimeHeader::setInReplyTo(const PString &str)
{
    MimeAddressField f(str);
    setField("in-reply-to", f.toString() );
}

void MimeHeader::setOrganization(const PString &str)
{
    MimeHeaderField f( str );
    setField("organization", f.toString() ); // No tr
}

void MimeHeader::setSubject(MimeHeaderField f)
{
    setField("subject", f.toString()); // No tr
}

void MimeHeader::setSubject(const PString &str)
{
    MimeHeaderField f( str );
    setField("subject", f.toString()); // No tr
}

void MimeHeader::setDate(MimeDateField f)
{
    setField("date", f.toString() ); // No tr
}

void MimeHeader::setContentType(MimeContentType f)
{
    setField("content-type", f.toString() );
}

void MimeHeader::setContentType(const PString &str)
{
    MimeContentType f = field("content-type");
    f.setContentType( str );
    setField("content-type", f.toString() );
}

void MimeHeader::setContentDisposition(MimeContentDisposition f)
{
    setField("content-disposition", f.toString() );
}

void MimeHeader::setContentDisposition(const PString &str)
{
    MimeContentType f = field("content-disposition");
    f.setContentType( str );
    setField("content-disposition", f.toString() );
}

// Adds user defined fields to the header.  Make sure they conform to
// the rfc definitions.
bool MimeHeader::setCustomField(const PString &f, const PString &bdy)
{
    // TODO This is a very simple test of field structure.  Need to improve this one
    PString strf = f.simplifyWhiteSpace().lower();
    if ( strf.find(" ", 0, false) > -1 )
        return false;
    if ( strf.find(":", 0, false) > -1 )
        return false;

    PString strbdy = bdy.simplifyWhiteSpace();

    setField( strf, strbdy);

    return true;
}

void MimeHeader::clear()
{
    _transferEncoding = EightBit;
    MimeHeaderBase::clear();
}

