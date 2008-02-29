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


#ifndef MIME_DATE_FIELD_H
#define MIME_DATE_FIELD_H

#include <qdatetime.h>

#include "pstring.h"
#include "mimeheaderfield.h"

class MimeDateField : public MimeHeaderField
{
public:
    MimeDateField();
    MimeDateField(const PString &s);
    MimeDateField(QDateTime d);
    ~MimeDateField();

    QDateTime dateTime() const;

protected:
    virtual void parse(const PString &);
    virtual PString encode();

private:
    QDateTime _dateTime;
};

#endif
