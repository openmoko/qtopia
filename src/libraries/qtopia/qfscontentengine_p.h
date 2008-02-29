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
#ifndef QFSCONTENTENGINE_P_H
#define QFSCONTENTENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qtopia/private/qcontentengine_p.h>

class QFSContentEngine : public QContentEngine
{
public:
    QFSContentEngine();
    virtual ~QFSContentEngine();

    virtual QContentEngine *copyTo( const QString &newPath );
    virtual bool moveTo( const QString &newPath );

    virtual bool execute( const QStringList &arguments ) const;

    virtual QIODevice *open( QIODevice::OpenMode mode );

    virtual QContentEngine *createCopy() const;

    virtual bool isOutOfDate() const;

    virtual bool remove();

protected:
    QFSContentEngine( const QString &engineType );

    static QString newFileName( const QString &name, const QString &type, const QString &location );
    static QString determineExtension( const QString &name, const QMimeType &type );

    virtual qint64 querySize();

    virtual bool queryValidity();
};

#endif
