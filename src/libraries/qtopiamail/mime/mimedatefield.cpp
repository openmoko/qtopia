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


#include "mimedatefield.h"
#include "mimetools.h"

MimeDateField::MimeDateField()
    : MimeHeaderField()
{
}

MimeDateField::MimeDateField(const PString &s)
    : MimeHeaderField(s)
{
    fromString(s);
}

MimeDateField::MimeDateField(QDateTime d)
{
    _dateTime = d;
}

MimeDateField::~MimeDateField()
{
}

QDateTime MimeDateField::dateTime() const
{
    return _dateTime;
}

void MimeDateField::parse(const PString &s)
{
    _dateTime = MimeTools::parseDate( s );
}

PString MimeDateField::encode()
{
    if ( !_dateTime.isNull() ) {
        return MimeTools::buildDateString(_dateTime);
    } else {
        return MimeHeaderField::encode();
    }
}

