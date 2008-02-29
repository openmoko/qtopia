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


#ifndef MIME_HEADER_BASE_H
#define MIME_HEADER_BASE_H

#include <qmap.h>

#include "pstring.h"

class MimeHeaderBase
{
public:
    MimeHeaderBase();
    MimeHeaderBase(const PString &s);
    virtual ~MimeHeaderBase();

    virtual void fromString(const PString &s);
    virtual PString toString() const;

    bool contains(const PString &s) const;
    PString field(const PString &) const;
    void setField(const PString &f, const PString &b);

    void clear();

protected:
    virtual void parse(const PString &s);

private:
    void parseHeaderBase(const PString &s);

private:
    QMap<PString,PString> fields;
};

inline bool MimeHeaderBase::contains(const PString &s) const
{
    return fields.contains(s);
}

inline PString MimeHeaderBase::field(const PString &s) const
{
    return *fields.find(s);
}

#endif
