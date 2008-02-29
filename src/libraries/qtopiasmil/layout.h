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

#include <qtopia/smil/element.h>
#include <qtopia/smil/module.h>
#include <qmap.h>

class SmilRootLayout : public SmilElement
{
public:
    SmilRootLayout(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts);

    void process();
    void paint(QPainter *p);

private:
    QSize size;
};

class SmilRegion : public SmilElement
{
public:
    SmilRegion(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts);

    void process();
    void setRect(const QRect &r);
    void paint(QPainter *p);

    void addReferrer(SmilElement *e);

    enum Fit { FillFit, HiddenFit, MeetFit, ScrollFit, SliceFit };
    enum ShowBackground { ShowAlways, ShowActive };

    struct Position {
        Position() : value(0), type(Unspecified) {}
        int value;
        enum Type { Unspecified, Absolute, Percent };
        Type type;
    };

    Position parsePosition(const QString &val);

    Fit fit;
    QString regionName;
    ShowBackground showBackground;
    int zIndex;
    Position left;
    Position right;
    Position width;
    Position top;
    Position bottom;
    Position height;
    SmilElementList referrers;
};

class SmilLayout : public SmilElement
{
public:
    SmilLayout(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts);
    virtual ~SmilLayout();
};

//===========================================================================

class SmilLayoutModule : public SmilModule
{
public:
    SmilLayoutModule();
    virtual ~SmilLayoutModule();

    virtual SmilElement *beginParseElement(SmilSystem *, SmilElement *, const QString &qName, const QXmlAttributes &atts);
    virtual bool parseAttributes(SmilSystem *sys, SmilElement *e, const QXmlAttributes &atts);
    virtual void endParseElement(SmilElement *, const QString &qName);
    virtual QStringList elements() const;
    virtual QStringList attributes() const;

    SmilRegion *findRegion(const QString &id);

protected:
    QList<SmilLayout*> layouts;
};

//===========================================================================

class RegionAttribute : public SmilModuleAttribute
{
public:
    RegionAttribute(SmilLayoutModule *m, SmilElement *e, const QXmlAttributes &atts);

    void process();

private:
    QString rgn;
    SmilLayoutModule *layoutModule;
};

