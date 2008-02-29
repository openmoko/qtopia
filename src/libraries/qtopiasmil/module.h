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

#ifndef MODULE_H
#define MODULE_H

#include <qstring.h>
#include <qstringlist.h>

// WARNING: These classes are not part of the Qtopia API.
// They are subject to change at any time.

class SmilElement;
class SmilSystem;
class SmilEvent;
class QXmlAttributes;
class QPainter;

class SmilModule
{
public:
    virtual ~SmilModule();
    virtual SmilElement *beginParseElement(SmilSystem *sys, SmilElement *parent, const QString &qName, const QXmlAttributes &) = 0;
    virtual bool parseAttributes(SmilSystem *sys, SmilElement *e, const QXmlAttributes &atts) = 0;
    virtual void endParseElement(SmilElement *parent, const QString &qName) = 0;
    virtual QStringList elements() const;
    virtual QStringList attributes() const;

    virtual void process();
};

class SmilModuleAttribute
{
public:
    SmilModuleAttribute(SmilElement *e, const QXmlAttributes &atts);
    virtual ~SmilModuleAttribute();

    virtual QString name() const;
    virtual void process();
    virtual void reset();
    virtual void event(SmilElement *, SmilEvent *);
    virtual void paint(QPainter *p);

    SmilElement *element;
};

#endif
