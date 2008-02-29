/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef STRUCTURE_H
#define STRUCTURE_H

#include <qtopia/smil/module.h>
#include <qtopia/smil/timing.h>


class Smil : public SmilElement
{
public:
    Smil(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts);
};

class SmilHead : public SmilElement
{
public:
    SmilHead(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts);
};

class SmilBody : public SmilSeq
{
public:
    SmilBody(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts);

    void setState(State);
};

//===========================================================================

class SmilStructureModule : public SmilModule
{
public:
    SmilStructureModule();

    virtual SmilElement *beginParseElement(SmilSystem *, SmilElement *, const QString &qName, const QXmlAttributes &);
    virtual bool parseAttributes(SmilSystem *sys, SmilElement *e, const QXmlAttributes &atts);
    virtual void endParseElement(SmilElement *, const QString &qName);
    virtual QStringList elements() const;
};

#endif

