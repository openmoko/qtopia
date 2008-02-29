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

#ifndef __QTOPIA_MEDIALIBRARY_QMEDIACONTENT_H
#define __QTOPIA_MEDIALIBRARY_QMEDIACONTENT_H

#include <qobject.h>
#include <qstring.h>
#include <qcontent.h>

#include "qmediahandle.h"

class QMediaContentPrivate;

class QTOPIAMEDIA_EXPORT QMediaContent : public QObject
{
    Q_OBJECT

    friend class QMediaContentPrivate;

public:
    explicit QMediaContent(QString const& url,
                           QString const& domain = QLatin1String("default"),
                           QObject* parent = 0);
    explicit QMediaContent(QContent const& content,
                           QString const& domain = QLatin1String("default"),
                           QObject* parent = 0);
    ~QMediaContent();

    QMediaHandle handle() const;

    QStringList controls() const;

signals:
    void controlAvailable(const QString& id);
    void controlUnavailable(const QString& id);
    void mediaError(const QString& mediaError);

private:
    QMediaContentPrivate*   d;
};


#endif  // __QTOPIA_MEDIALIBRARY_QMEDIACONTENT_H
