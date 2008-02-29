/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef XMLREADER_H
#define XMLREADER_H

#include <qtopia/qpeglobal.h>
#include <qtopia/categories.h>
#include <qstring.h>
#include <qxml.h>
#include <qmap.h>

class QTOPIA_EXPORT Node
{
public:
    Node();
    ~Node();

    void addChild( Node *child );

    void setAttributes( const QXmlAttributes &a );
    QMap<QString, QString> attributeMap();
    QString attribute( const QString& name );

    Node *nextNode() const { return next; }
    Node *prevNode() const { return prev; }
    Node *parentNode() const { return parent; }
    Node *lastChild() const { return last; }
    Node *firstChild() const { return first; }

    void setTagName( const QString &s ) { tagN = s; }
    QString tagName() const { return tagN; }
    void setData( const QString &s ) { dt = s; }
    QString data() const { return dt; }
    QString subData(const QString& tag) const;
    void appendData( const QString s ) { dt += s; }


private:
    QMap<QString, QString> attributes;
    QString dt, tagN;

    Node *parent, *prev, *next, *first, *last;
};

class XmlHandlerPrivate;
class QTOPIA_EXPORT XmlHandler : public QXmlDefaultHandler
{
public:
    XmlHandler();
    ~XmlHandler();

    bool startDocument();
    bool endDocument();
    bool startElement( const QString &, const QString &, const QString &qName,
		       const QXmlAttributes &attr );
    bool endElement( const QString &, const QString &, const QString & );
    bool characters( const QString &ch );

    Node *firstNode() const { return tree; }

private:
    Node *node, *tree;
    XmlHandlerPrivate *d;
};



#endif
