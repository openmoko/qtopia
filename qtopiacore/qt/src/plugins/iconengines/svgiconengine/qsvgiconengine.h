/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSVGICONENGINE_H
#define QSVGICONENGINE_H

#include <QtGui/qiconengine.h>
#include <QtCore/qshareddata.h>

class QSvgIconEnginePrivate;

class QSvgIconEngine : public QIconEngine
{
public:
    QSvgIconEngine();
    virtual ~QSvgIconEngine();
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
    QSharedDataPointer<QSvgIconEnginePrivate> d;
};

#endif
