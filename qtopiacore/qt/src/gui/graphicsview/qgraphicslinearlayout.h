/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QGRAPHICSLINEARLAYOUT_H
#define QGRAPHICSLINEARLAYOUT_H

#include <QtGui/qgraphicsitem.h>
#include <QtGui/qgraphicslayout.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#if !defined(QT_NO_GRAPHICSVIEW) || (QT_EDITION & QT_MODULE_GRAPHICSVIEW) != QT_MODULE_GRAPHICSVIEW

class QGraphicsLinearLayoutPrivate;

class Q_GUI_EXPORT QGraphicsLinearLayout : public QGraphicsLayout
{
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(qreal spacing READ spacing WRITE setSpacing)
public:
    QGraphicsLinearLayout(QGraphicsLayoutItem *parent = 0);
    QGraphicsLinearLayout(Qt::Orientation orientation, QGraphicsLayoutItem *parent = 0);
    virtual ~QGraphicsLinearLayout();

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

    inline void addItem(QGraphicsLayoutItem *item) { insertItem(-1, item); }
    inline void addStretch(int stretch = 1) { insertStretch(-1, stretch); }

    void insertItem(int index, QGraphicsLayoutItem *item);
    void insertStretch(int index, int stretch = 1);

    void removeItem(QGraphicsLayoutItem *item);
    void removeAt(int index);

    void setSpacing(qreal spacing);
    qreal spacing() const;
    void setItemSpacing(int index, qreal spacing);
    qreal itemSpacing(int index) const;

    void setStretchFactor(QGraphicsLayoutItem *item, int stretch);
    int stretchFactor(QGraphicsLayoutItem *item) const;

    void setAlignment(QGraphicsLayoutItem *item, Qt::Alignment alignment);
    Qt::Alignment alignment(QGraphicsLayoutItem *item) const;

    void setGeometry(const QRectF &rect);

    int count() const;
    QGraphicsLayoutItem *itemAt(int index) const;

    void invalidate();
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

#if 0 // ###
    Q5SizePolicy::ControlTypes controlTypes(LayoutSide side) const;
#endif

#ifdef QT_DEBUG
    void dump(int indent = 0) const;
#endif

protected:
#if 0
    QSize contentsSizeHint(Qt::SizeHint which, const QSize &constraint = QSize()) const;
#endif

private:
    Q_DISABLE_COPY(QGraphicsLinearLayout)
    Q_DECLARE_PRIVATE(QGraphicsLinearLayout)
};

#endif

QT_END_NAMESPACE

QT_END_HEADER

#endif

