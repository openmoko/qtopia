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

#ifndef QSVGFONT_P_H
#define QSVGFONT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qpainterpath.h"
#include "qhash.h"
#include "qstring.h"
#include "qsvgstyle_p.h"

class QSvgGlyph
{
public:
    QSvgGlyph(QChar unicode, const QPainterPath &path, qreal horizAdvX);
    QSvgGlyph() : m_unicode(0), m_horizAdvX(0) {}

    QChar m_unicode;
    QPainterPath m_path;
    qreal m_horizAdvX;
};


class QSvgFont : public QSvgRefCounted
{
public:
    QSvgFont(qreal horizAdvX);

    void setFamilyName(const QString &name);
    QString familyName() const;

    void setUnitsPerEm(qreal upem);

    void addGlyph(QChar unicode, const QPainterPath &path, qreal horizAdvX = -1);

    void draw(QPainter *p, const QPointF &point, const QString &str, qreal pixelSize) const;
public:
    QString m_familyName;
    qreal m_unitsPerEm;
    qreal m_ascent;
    qreal m_descent;
    qreal m_horizAdvX;
    QHash<QChar, QSvgGlyph> m_glyphs;
};

#endif
