/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include "content.h"

SmilPrefetch::SmilPrefetch(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts)
    : SmilElement(sys, p, n, atts)
{
}

SmilSwitch::SmilSwitch(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts)
    : SmilElement(sys, p, n, atts)
{
}

//===========================================================================

SmilContentModule::SmilContentModule()
    : SmilModule()
{
}

SmilContentModule::~SmilContentModule()
{
}

SmilElement *SmilContentModule::beginParseElement(SmilSystem *sys, SmilElement *e, const QString &qName, const QXmlAttributes &atts)
{
    if (qName == "switch") {
        return new SmilSwitch(sys, e, qName, atts);
    } else if (qName == "prefetch") {
        return new SmilPrefetch(sys, e, qName, atts);
    }

    return 0;
}

bool SmilContentModule::parseAttributes(SmilSystem *, SmilElement *, const QXmlAttributes &)
{
    return false;
}

void SmilContentModule::endParseElement(SmilElement *, const QString &)
{
    //### we need to process now, and remove element where necessary.
}

QStringList SmilContentModule::elements() const
{
    QStringList l;
    l.append("switch");
    l.append("prefetch");
    return l;
}

