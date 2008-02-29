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

#include <qtopia/smil/element.h>
#include <qtopia/smil/module.h>

// WARNING: These classes are not part of the Qtopia API.
// They are subject to change at any time.

class SmilPrefetch : public SmilElement
{
public:
    SmilPrefetch(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts);
};

class SmilSwitch : public SmilElement
{
public:
    SmilSwitch(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts);
};

//===========================================================================

class SmilContentModule : public SmilModule
{
public:
    SmilContentModule();
    virtual ~SmilContentModule();

    virtual SmilElement *beginParseElement(SmilSystem *, SmilElement *, const QString &qName, const QXmlAttributes &atts);
    virtual bool parseAttributes(SmilSystem *sys, SmilElement *e, const QXmlAttributes &atts);
    virtual void endParseElement(SmilElement *, const QString &qName);
    virtual QStringList elements() const;
};

