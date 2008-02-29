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


#ifndef MIME_MESSAGE_H
#define MIME_MESSAGE_H

#include "pstring.h"
#include "mimeentity.h"

class MimeMessage : public MimeEntity
{
public:
    MimeMessage();
    MimeMessage(const PString &s);
    MimeMessage(const MimeMessage &);

    virtual ~MimeMessage();

    virtual void fromString(const PString &s);
    virtual PString toString() const;

    bool textOnly() const;
    PString bodyText(bool html = false) const;

    void clear();

protected:
    virtual void setPreamble(const PString &);
    virtual void setEpilogue(const PString &);

    virtual void parse(const PString &s);

private:
    // won't work withouth impl.  Disallow it for now
    MimeMessage operator=(const MimeMessage &);

private:
    PString _preamble, _epilogue;
};

#endif
