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

#include "structure.h"
#include "element.h"
#include "system.h"


Smil::Smil(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts)
    : SmilElement(sys, p, n, atts)
{
    currentState = Active;
}

SmilHead::SmilHead(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts)
    : SmilElement(sys, p, n, atts)
{
    currentState = Active;
    vis = true;
}

SmilBody::SmilBody(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts)
    : SmilSeq(sys, p, n, atts)
{
}

void SmilBody::setState(State s)
{
    SmilSeq::setState(s);
    if (s == Post) {
        sys->bodyFinished();
    }
}

//===========================================================================

SmilStructureModule::SmilStructureModule() : SmilModule()
{
}

SmilElement *SmilStructureModule::beginParseElement(SmilSystem *sys, SmilElement *parent, const QString &qName, const QXmlAttributes &atts)
{
    if (qName == "smil") {
        if (parent) {
            qWarning("smil element should be root");
        } else {
            return new Smil(sys, parent, qName, atts);
        }
    } else if (qName == "head") {
        return new SmilHead(sys, parent, qName, atts);
    } else if (qName == "body") {
        return new SmilBody(sys, parent, qName, atts);
    }

    return 0;
}

bool SmilStructureModule::parseAttributes(SmilSystem *, SmilElement *, const QXmlAttributes &)
{
    return false;
}

void SmilStructureModule::endParseElement(SmilElement *, const QString &)
{
}

QStringList SmilStructureModule::elements() const
{
    QStringList l;
    l.append("smil");
    l.append("head");
    l.append("body");
    return l;
}


