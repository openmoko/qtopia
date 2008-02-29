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


#ifndef MIME_CONTENT_DISP_H
#define MIME_CONTENT_DISP_H

#include "pstring.h"
#include "mimeheaderfield.h"

class MimeContentDisposition : public MimeHeaderField
{
public:
    MimeContentDisposition();
    MimeContentDisposition(const PString &s);
    ~MimeContentDisposition();

    PString contentDisposition() const;
    PString filename() const;

    void setContentDisposition(const PString &);
    void setFilename(const PString &);

protected:
    virtual void parse(const PString &);
    virtual PString encode();

    void parseContentDisposition(const PString &s);


private:
    PString _contentDisposition;
    PString _filename;
};

#endif
