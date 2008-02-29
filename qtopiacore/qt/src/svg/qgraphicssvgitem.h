/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef QGRAPHICSSVGITEM_H
#define QGRAPHICSSVGITEM_H

#include <QtGui/qgraphicsitem.h>
#include <QtCore/qobject.h>

#ifndef QT_NO_GRAPHICSVIEW

QT_BEGIN_HEADER

QT_MODULE(Svg)

class QSvgRenderer;
class QGraphicsSvgItemPrivate;

class Q_SVG_EXPORT QGraphicsSvgItem : public QObject, public QGraphicsItem
{
    Q_OBJECT

public:
    QGraphicsSvgItem(QGraphicsItem *parentItem=0);
    QGraphicsSvgItem(const QString &fileName, QGraphicsItem *parentItem=0);

    void setSharedRenderer(QSvgRenderer *renderer);
    QSvgRenderer *renderer() const;

    void setElementId(const QString &id);
    QString elementId() const;

    void setCachingEnabled(bool);
    bool isCachingEnabled() const;

    void setMaximumCacheSize(const QSize &size);
    QSize maximumCacheSize() const;

    virtual QRectF boundingRect() const;

    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget=0);

    enum { Type = 13 };
    virtual int type() const;

private:
    Q_DISABLE_COPY(QGraphicsSvgItem)

    // Q_DECLARE_PRIVATE_WITH_BASE(QGraphicsSvgItem, QObject)
    inline QGraphicsSvgItemPrivate *d_func()
    { return reinterpret_cast<QGraphicsSvgItemPrivate *>(QObject::d_ptr); }
    inline const QGraphicsSvgItemPrivate *d_func() const
    { return reinterpret_cast<const QGraphicsSvgItemPrivate *>(QObject::d_ptr); }
    friend class QGraphicsSvgItemPrivate;

    Q_PRIVATE_SLOT(d_func(), void _q_repaintItem())
};

QT_END_HEADER

#endif // QT_NO_GRAPHICSVIEW
#endif // QGRAPHICSSVGITEM_H
