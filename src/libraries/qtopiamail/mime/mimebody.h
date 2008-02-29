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


#ifndef MIME_BODY_H
#define MIME_BODY_H

#include <qcstring.h>

#include "pstring.h"
#include "mimecontenttype.h"
#include "mimecontentdisp.h"

class MimeEntity;
class MimeBodyPrivate;
class MimeBodyPart;
class MimeMessage;

class MimeBody
{
public:
    MimeBody(MimeEntity *parent);
    MimeBody(MimeEntity *parent, const PString &s);
    MimeBody(const MimeBody &);
    virtual ~MimeBody();

    void fromString(const PString &s);
    PString toString();

    uint count() const;
    bool textOnly() const;

    MimeBodyPart *multiPartAt(uint i) const;
    MimeMessage *message() const;

    PString text() const;
    QByteArray data();

    bool removeMultiPartAt(uint i);
    bool removeMessage();

    bool setText(const PString &);
    bool setMessage(MimeMessage *);
    bool addMultiPart(const MimeContentType &ct, const MimeContentDisposition &cd, const PString &bdy);
    bool addMultiPart(const MimeContentType &ct, const MimeContentDisposition &cd, QByteArray &bdy);

    void clear();

    void reparent(MimeEntity *p) {_parent = p;};

protected:
    virtual void parse(const PString &s);

private:
    // won't work withouth impl.  Disallow it for now
    MimeBody operator=(const MimeBody &);

    void parseBody(const PString &s);
    int nextBoundaryPos(const PString &boundary, const PString &s, int start);
    void addBodyPart( const PString &s);

private:
    MimeEntity *_parent;
    MimeBodyPrivate *d;

    PString _txtBody;
};

#endif
