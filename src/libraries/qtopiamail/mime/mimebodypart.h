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


#ifndef MIME_BODY_PART_H
#define MIME_BODY_PART_H

#include "pstring.h"
#include "mimeentity.h"

class MimeBody;

class MimeBodyPart : public MimeEntity
{
public:
    MimeBodyPart(MimeEntity *parent, const PString &s);
    MimeBodyPart(MimeEntity *parent);
    MimeBodyPart(const MimeBodyPart &);
    virtual ~MimeBodyPart();

    virtual void fromString(const PString &s);
    virtual PString toString() const;

protected:
    virtual void parse(const PString &s);

    // We don't use this as som mailers can get confused
    virtual void setPreamble(const PString &) {};
    virtual void setEpilogue(const PString &) {};

private:
    // won't work withouth impl.  Disallow it for now
    MimeBodyPart operator=(const MimeBodyPart &);

    void parseBodyPart(const PString &s);

private:
    PString _preamble, _epilogue;
};

#endif
