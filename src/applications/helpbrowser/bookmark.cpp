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

#include "bookmark.h"

#include <QString>
#include <QDomNode>
#include <QDomDocument>
#include <QDomElement>

#include <QDebug>

/*!
  \internal
  \class Bookmark
  \brief An instance of Bookmark contains both a document title and a URL.

  \mainclass
  The Bookmark is designed to be contained within a UI list.

  \sa BookmarksUI
*/

/*! \fn Bookmark::Bookmark(const QUrl &_url,const QString &title,QObject *parent)
  Creates a Bookmark with the given URL and title.
*/
Bookmark::Bookmark(const QUrl &_url,const QString &_title,QObject *parent) :
    QObject(parent)
    , title(_title)
    , url(_url)
{
}

/*! \fn Bookmark::Bookmark()
  Required for compatibility with generic collections and for QVariant wrapper class.
  Can also be used to construct an 'invalid' Bookmark, i.e. a Bookmark with an empty Url.
*/

/*! \fn Bookmark::Bookmark(const Bookmark &)
  Copy ctor, required for compatibility with QVariant wrapper class.
*/

/*! \fn Bookmark::Bookmark(const QDomElement &,QObject *parent)
  Creates a Bookmark using information from the given XML element, which should conform
  to the element produced by \l{function}{appendAsXML(QDomDocument &,QDomNode &) const}.
*/
Bookmark::Bookmark(const QDomElement &node,QObject *parent)
    : QObject(parent)
    , title(node.attribute("Title"))
    , url(node.attribute("Url"))
{
}

Bookmark::Bookmark(const Bookmark &bookmark)
    : QObject()
    , title(bookmark.getTitle())
    , url(bookmark.url)
{
}

/*!  \fn QString Bookmark::getTitle() const
  Retrieves the document title.
*/

/*! \fn QUrl &Bookmark::getUrl() const
  Retrieves the URL.
*/

/*! \fn void Bookmark::appendAsXML(QDomDocument &,QDomNode &) const
  Creates an XML element that describes this Bookmark, and appends it to the given document.
*/
void Bookmark::appendAsXML(QDomDocument &doc,QDomNode &node) const
{
    QDomElement elem = doc.createElement("Bookmark");
    node.appendChild(elem);

    // Add the title and the URL.
    elem.setAttribute("Title",title);
    elem.setAttribute("Url",url.toString());
}

/* \fn bool operator==(const Bookmark& b1,const Bookmark& b2)
  Two Bookmark objects are equal if both their titles and their Urls are equal.
*/
bool operator==(const Bookmark& b1,const Bookmark& b2)
{
    return ( (b1.getTitle() == b2.getTitle()) && (b1.url == b2.url) );
}


