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

#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <QVariant>
#include <QUrl>

class QString;
class QListWidget;
class QDomNode;
class QDomDocument;
class QDomElement;


class Bookmark : public QObject
{
public:

    // Required for compatibility with generic collections and for QVariant
    Bookmark() {}

    // Required for QVariant.
    Bookmark(const Bookmark &);

    Bookmark(const QUrl &url,const QString &title,QObject *parent = 0);

    Bookmark(const QDomElement &,QObject *parent = 0);

    QString getTitle() const { return title; }

    const QUrl &getUrl() const { return url; }

    void appendAsXML(QDomDocument &,QDomNode &) const;

    friend bool operator==(const Bookmark& b1,const Bookmark& b2);

private:

    QString title;
    QUrl url;
};

Q_DECLARE_METATYPE(Bookmark)

#endif
