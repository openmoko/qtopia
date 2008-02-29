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

#ifndef QTHUMBNAIL_H
#define QTHUMBNAIL_H

#include <qtopiaglobal.h>

#include <QString>
#include <QSize>
#include <QPixmap>

class QThumbnailPrivate;

class QTOPIA_EXPORT QThumbnail
{
public:
    explicit QThumbnail( const QString& fileName );
    explicit QThumbnail( QIODevice *device );

    ~QThumbnail();

    QSize actualSize( const QSize& size = QSize(), Qt::AspectRatioMode mode = Qt::KeepAspectRatio );

    QPixmap pixmap( const QSize& size, Qt::AspectRatioMode mode = Qt::KeepAspectRatio );

private:
    QThumbnailPrivate *d;
};

#endif // QTHUMBNAIL_H
