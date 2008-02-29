/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef SVGICONENGINE_H
#define SVGICONENGINE_H

#include <QtGui/qiconengine.h>
#include <QtCore/qshareddata.h>

class QtopiaSvgIconEnginePrivate;

class QtopiaSvgIconEngine : public QIconEngine
{
public:
    QtopiaSvgIconEngine();
    virtual ~QtopiaSvgIconEngine();
    virtual void paint(QPainter *painter, const QRect &rect,
                       QIcon::Mode mode, QIcon::State state);
    virtual QSize actualSize(const QSize &size, QIcon::Mode mode,
                             QIcon::State state);
    virtual QPixmap pixmap(const QSize &size, QIcon::Mode mode,
                           QIcon::State state);

    virtual void addPixmap(const QPixmap &pixmap, QIcon::Mode mode,
                           QIcon::State state);
    virtual void addFile(const QString &fileName, const QSize &size,
                         QIcon::Mode mode, QIcon::State state);
private:
    QSharedDataPointer<QtopiaSvgIconEnginePrivate> d;
};

#endif
