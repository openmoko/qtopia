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


#ifndef MIME_HEADER_H
#define MIME_HEADER_H

#include <qvaluelist.h>

#include "pstring.h"
#include "mimeheaderbase.h"
#include "mimeaddressfield.h"
#include "mimecontenttype.h"
#include "mimecontentdisp.h"
#include "mimedatefield.h"

class MimeEntity;

namespace Mime {
    enum TransferEncoding {
        EightBit, Base64, QuotedPrintable
    };
};

class MimeHeader : public MimeHeaderBase
{
public:
    MimeHeader(MimeEntity *parent);
    MimeHeader(MimeEntity *parent, const PString &s);
    virtual ~MimeHeader();

    MimeContentType contentType() const;
    MimeContentDisposition contentDisposition() const;

    MimeAddressField from() const;
    MimeAddressField replyTo() const;
    MimeAddressField inReplyTo() const;

    MimeHeaderField organization() const;

    MimeHeaderField subject() const;
    MimeHeaderField messageId() const;

    MimeDateField date() const;

    QValueList<MimeAddressField> to() const;
    QValueList<MimeAddressField> cc() const;
    QValueList<MimeAddressField> bcc() const;

    Mime::TransferEncoding transferEncoding() const;
    void setTransferEncoding(Mime::TransferEncoding te);

    void setTo(QValueList<MimeAddressField> l);
    void setCc(QValueList<MimeAddressField> l);
    void setBcc(QValueList<MimeAddressField> l);

    void setFrom(MimeAddressField f);
    void setFrom(const PString &);

    void setReplyTo(MimeAddressField &);
    void setReplyTo(const PString &);

    void setInReplyTo(MimeAddressField &);
    void setInReplyTo(const PString &);

    void setOrganization(const PString &);

    void setSubject(MimeHeaderField f);
    void setSubject(const PString &);

    void setDate(MimeDateField f);

    void setContentType(MimeContentType f);
    void setContentType(const PString &);

    void setContentDisposition(MimeContentDisposition f);
    void setContentDisposition(const PString &);

    bool setCustomField(const PString &f, const PString &bdy);

    void reparent(MimeEntity *p) {_parent = p;};
    void clear();

protected:
    virtual void parse(const PString &s);

    static Mime::TransferEncoding transferEncodingId( const PString &str );

private:
    QValueList<MimeAddressField> asList(const PString &s) const;
    PString fromList(QValueList<MimeAddressField> &) const;

private:
    MimeEntity *_parent;

    Mime::TransferEncoding _transferEncoding;
};

#endif
