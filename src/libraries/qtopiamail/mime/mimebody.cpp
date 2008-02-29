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


#include <qlist.h>

#include "mimeentity.h"
#include "mimebody.h"
#include "mimebodypart.h"
#include "mimeheader.h"
#include "mimemessage.h"
#include "mimetools.h"

static int level = 0;

class MimeBodyPrivate
{
public:
    MimeBodyPrivate()
    {
        message = 0;
        bodyParts = 0;
    }

    // copy constructor
    MimeBodyPrivate(MimeBodyPrivate *d)
    {
        message = 0;
        bodyParts = 0;

        if ( d->message )
            message = new MimeMessage( *d->message );

        MimeBodyPart *bp;
        if ( d->bodyParts ) {
            createList();

            QListIterator<MimeBodyPart> it(*d->bodyParts);
            for ( ; it.current(); ++it ) {
                // Using *it with gcc fails casting to MimeBodyPart (copy constructor fails, even when explicitly stating it)
                MimeBodyPart *g = it.current();
                bp = new MimeBodyPart( *g );
                bodyParts->append( bp );
            }
        }
    }

    ~MimeBodyPrivate()
    {
        clear();
        delete bodyParts;
    }

    uint count()
    {
        if ( !bodyParts )
            return 0;

        return bodyParts->count();
    }

    MimeBodyPart* at(uint i)
    {
        if ( !bodyParts )
            return NULL;

        return bodyParts->at(i);
    }

    void append(MimeBodyPart *m)
    {
        createList();
        bodyParts->append(m);
    }

    bool remove(uint i)
    {
        if ( !bodyParts )
            return false;

        return bodyParts->remove(i);
    }

    void clear()
    {
        if ( bodyParts )
            bodyParts->clear();

        if ( message ) {
            delete message;
            message = 0;
        }
    }

    MimeMessage *message;

private:
    void createList()
    {
        if ( bodyParts )
            return;

        bodyParts = new QList<MimeBodyPart>;
        bodyParts->setAutoDelete( true );
    }

    QList<MimeBodyPart> *bodyParts;
};

MimeBody::MimeBody(MimeEntity *parent)
    : _parent(parent)
{
    d  = new MimeBodyPrivate();
}

MimeBody::MimeBody(MimeEntity *parent, const PString &s)
    : _parent(parent)
{
    d  = new MimeBodyPrivate();

    fromString(s);
}

MimeBody::MimeBody(const MimeBody &other)
    : _parent(0)
{
    _txtBody = other._txtBody;
    d = new MimeBodyPrivate(other.d);
}

MimeBody::~MimeBody()
{
    delete d;
}

void MimeBody::fromString(const PString &s)
{
    parse(s);
}

PString MimeBody::toString()
{
    if ( d->count() > 0 ) {
        PString str;
        PString boundary = _parent->header()->contentType().boundary();
        for (uint i = 0; i < d->count(); i++) {
            str += d->at(i)->toString();

            if ( i + 1 < d->count() )
                str += "\n--" + boundary + "\n";
        }

        return str;
    } else if ( d->message ) {
        return d->message->toString();
    }

    return _txtBody;
}

uint MimeBody::count() const
{
    return d->count();
}

bool MimeBody::textOnly() const
{
    return (!count() && !message());
}

MimeBodyPart* MimeBody::multiPartAt(uint i) const
{
    return d->at(i);
}

bool MimeBody::removeMultiPartAt(uint i)
{
    return d->remove(i);
}

bool MimeBody::removeMessage()
{
    if ( d->message ) {
        delete d->message;
        d->message = 0;
        return true;
    }
    return false;
}


MimeMessage* MimeBody::message() const
{
    return d->message;
}

//  Decodes the text according to the encoded transport mechanism
PString MimeBody::text() const
{
    if ( _parent->header()->transferEncoding() == Mime::Base64 ) {
        return PString( MimeTools::decodeBase64( _txtBody ) );
    } else if ( _parent->header()->transferEncoding() == Mime::QuotedPrintable ) {
        return MimeTools::decodeQuotedPrintable(_txtBody );
    } else {
        return _txtBody;
    }
}

//  Decodes the text according to the encoded transport mechanism
QByteArray MimeBody::data()
{
    QByteArray b;

    if ( _parent->header()->transferEncoding() == Mime::Base64 ) {
        b = MimeTools::decodeBase64( _txtBody );
    } else if ( _parent->header()->transferEncoding() == Mime::QuotedPrintable ) {
        PString tmp = MimeTools::decodeQuotedPrintable(_txtBody);
        b.duplicate( tmp.data(), tmp.length() );
    } else {
        b.duplicate( _txtBody.data(), _txtBody.length() );
    }

    return b;
}

bool MimeBody::setText(const PString &str)
{
    if ( d->message || d->count() )
        return false;

    _txtBody = str;
    return true;
}

bool MimeBody::setMessage(MimeMessage *m)
{
    if ( d->count() || !_txtBody.isEmpty() || d->message )
        return false;

    if ( m == _parent ) {
        qWarning("MimeMessage: this* cannot contain this*");
        return false;
    }

    MimeContentType pf;
    pf.setContentType( "messaget/rfc822" );
    pf.createBoundary(++level);

    _parent->_header->setContentType( pf );
    _parent->setPreamble( "This is a multipart message in Mime 1.0 format\n" );

    d->message = m;
    d->message->_parent = _parent;

    return true;
}

bool MimeBody::addMultiPart(const MimeContentType &ct, const MimeContentDisposition &cd, const PString &bdy)
{
    MimeContentType pf;
    pf.setContentType( "multipart/mixed" );
    pf.createBoundary(++level);

    _parent->_header->setContentType( pf );
    _parent->setPreamble( "This is a multipart message in Mime 1.0 format\n" );

    MimeBodyPart *m = new MimeBodyPart( _parent );
    m->_header->setContentType( ct );
    m->_header->setContentDisposition( cd );
    m->_body->setText( bdy + "\n" );

    d->append( m );

    return true;
}

bool MimeBody::addMultiPart(const MimeContentType &ct, const MimeContentDisposition &cd, QByteArray &bdy)
{
    MimeContentType pf;
    pf.setContentType( "multipart/mixed" );
    pf.createBoundary(++level);

    _parent->_header->setContentType( pf );
    _parent->setPreamble( "This is a multipart message in Mime 1.0 format\n" );

    MimeBodyPart *m = new MimeBodyPart( _parent );
    MimeHeader *h = m->_header;
    h->setContentType( ct );
    h->setContentDisposition( cd );

    h->setTransferEncoding( Mime::Base64 );

    m->_body->setText( MimeTools::encodeBase64(bdy) + "\n");

    d->append( m );

    return true;
}

void MimeBody::clear()
{
    d->clear();
    _txtBody = "";
}

void MimeBody::parse(const PString &s)
{
    d->clear();
    _txtBody = "";

    parseBody( s );
}

/*  We don't do a single pass parse, instead we find the end boundary before we break
    it down further.  For single pass we would need also need to test for parent-boundaries until
    we get the end delimiter.  The speed difference is neglible, and often faster with the current
    approach.
*/
int MimeBody::nextBoundaryPos(const PString &boundary, const PString &s, int start)
{
    int pos = s.find(boundary, start);
    if ( pos != -1 )
        return pos;

    if ( ( pos = s.find(boundary + "--", start) ) > -1 )
        return pos;

    // giving up on broken message.  Return length which is probably ok
    return s.length();
}

//  Body part found
void MimeBody::addBodyPart( const PString &s)
{
    MimeBodyPart *b = new MimeBodyPart(_parent, s );
    d->append( b );
}

void MimeBody::parseBody(const PString &s)
{
    //now we need to know what type of body we are
    MimeContentType ct = _parent->header()->contentType();

    PString type = ct.contentType();
    PString boundary = "\n--" + ct.boundary();

    int msgStart = 0;
    if ( type == "multipart/mixed" || type == "multipart/parallel" ||
            type == "multipart/alternative" ) {

        while ( (uint) msgStart < s.length() ) {
            int end = nextBoundaryPos(boundary, s, msgStart );

            PString str = s.mid(msgStart, end - msgStart);
            if ( str.stripWhiteSpace() != "" ) {
                addBodyPart( str );
            } else
                qWarning("empty multipart header definition");

            // get next line (we always discard the first boundary)
            msgStart = end + boundary.length();
            if ( (msgStart = s.find("\n", msgStart, false) ) == -1 )
                msgStart = s.length();
            else msgStart++;
        }

    } else if ( type == "message/rfc822" || type == "message/partial" ) {
        d->message = new MimeMessage( s.data() );
    } else {
        _txtBody = s;
    }
}

