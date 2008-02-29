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

#ifndef __QTOPIA_MEDIALIBRARY_QMEDIACONTENT_H
#define __QTOPIA_MEDIALIBRARY_QMEDIACONTENT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QContent>

#include "qmediahandle_p.h"


class QMediaAbstractControl;


class QMediaContentPrivate;

class QTOPIAMEDIA_EXPORT QMediaContent : public QObject
{
    Q_OBJECT

    friend class QMediaContentPrivate;
    friend class QMediaAbstractControl;

public:
    explicit QMediaContent(QUrl const& url,
                           QString const& domain = QLatin1String("Media"),
                           QObject* parent = 0);
    explicit QMediaContent(QContent const& content,
                           QString const& domain = QLatin1String("Media"),
                           QObject* parent = 0);
    ~QMediaContent();

    QStringList controls() const;

    static QStringList supportedMimeTypes();
    static QStringList supportedUriSchemes(QString const& mimeType);

    static void playContent(QUrl const& url, QString const& domain = "Media");
    static void playContent(QContent const& content, QString const& domain = "Media");

signals:
    void controlAvailable(const QString& name);
    void controlUnavailable(const QString& name);
    void mediaError(const QString& mediaError);

private:
    Q_DISABLE_COPY(QMediaContent);

    QMediaHandle handle() const;

    QMediaContentPrivate*   d;
};


#endif  // __QTOPIA_MEDIALIBRARY_QMEDIACONTENT_H

