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

#ifndef QGRAPHICSITEM_P_H
#define QGRAPHICSITEM_P_H

#include <QtGui/qmatrix.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#include "qgraphicsitem.h"

#if !defined(QT_NO_GRAPHICSVIEW) || (QT_EDITION & QT_MODULE_GRAPHICSVIEW) != QT_MODULE_GRAPHICSVIEW

class QGraphicsItemPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsItem)
public:
    enum Extra {
        ExtraMatrix,
        ExtraToolTip,
        ExtraCursor
    };

    inline QGraphicsItemPrivate()
        : z(0), scene(0), parent(0), index(-1), q_ptr(0)
    {
        acceptedMouseButtons = 0x1f;
        visible = 1;
        enabled = 1;
        selected = 0;
        acceptsHover = 0;
        acceptDrops = 0;
        isMemberOfGroup = 0;
        handlesChildEvents = 0;
        ancestorHandlesChildEvents = 0;
        itemDiscovered = 0;
        hasMatrix = 0;
        hasCursor = 0;
        flags = 0;
        pad = 0;
    }

    inline virtual ~QGraphicsItemPrivate()
    { }

    void setAncestorHandlesChildEvents(bool enabled);
    void setIsMemberOfGroup(bool enabled);
    void remapItemPos(QEvent *event, QGraphicsItem *item);
    
    inline QVariant extra(Extra type) const
    {
        for (int i = 0; i < extras.size(); ++i) {
            const ExtraStruct &extra = extras.at(i);
            if (extra.type == type)
                return extra.value;
        }
        return QVariant();
    }

    inline void setExtra(Extra type, const QVariant &value)
    {
        int index = -1;
        for (int i = 0; i < extras.size(); ++i) {
            if (extras.at(i).type == type) {
                index = i;
                break;
            }
        }

        if (index == -1) {
            extras << ExtraStruct(type, value);
        } else {
            extras[index].value = value;
        }
    }

    inline void unsetExtra(Extra type)
    {
        for (int i = 0; i < extras.size(); ++i) {
            if (extras.at(i).type == type) {
                extras.removeAt(i);
                return;
            }
        }
    }

    struct ExtraStruct {
        ExtraStruct(Extra type, QVariant value)
            : type(type), value(value)
        { }

        Extra type;
        QVariant value;

        bool operator<(Extra extra) const
        { return type < extra; }
    };
    QList<ExtraStruct> extras;

    QPointF pos;
    qreal z;
    QGraphicsScene *scene;
    QGraphicsItem *parent;
    QList<QGraphicsItem *> children;
    int index;
    quint32 acceptedMouseButtons : 5;
    quint32 visible : 1;
    quint32 enabled : 1;
    quint32 selected : 1;
    quint32 acceptsHover : 1;
    quint32 acceptDrops : 1;
    quint32 isMemberOfGroup : 1;
    quint32 handlesChildEvents : 1;
    quint32 ancestorHandlesChildEvents : 1;
    quint32 itemDiscovered : 1;
    quint32 hasMatrix : 1;
    quint32 hasCursor : 1;
    quint32 flags : 11;
    quint32 pad : 5;

    QGraphicsItem *q_ptr;
};

#endif // QT_NO_GRAPHICSVIEW

#endif
