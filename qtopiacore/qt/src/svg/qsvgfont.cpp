/****************************************************************************
**
** Copyright (C) 2005-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtSVG module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsvgfont_p.h"

#include "qpainter.h"
#include "qpen.h"
#include "qdebug.h"

QSvgGlyph::QSvgGlyph(QChar unicode, const QPainterPath &path, qreal horizAdvX)
    : m_unicode(unicode), m_path(path), m_horizAdvX(horizAdvX)
{

}


QSvgFont::QSvgFont(qreal horizAdvX)
    : m_horizAdvX(horizAdvX)
{
}


QString QSvgFont::familyName() const
{
    return m_familyName;
}


void QSvgFont::addGlyph(QChar unicode, const QPainterPath &path, qreal horizAdvX )
{
    m_glyphs.insert(unicode, QSvgGlyph(unicode, path,
                                       (horizAdvX==-1)?m_horizAdvX:horizAdvX));
}


void QSvgFont::draw(QPainter *p, const QPointF &point, const QString &str, qreal pixelSize) const
{
    p->save();
    p->translate(point);
    p->scale(pixelSize/m_unitsPerEm, -pixelSize/m_unitsPerEm);

    // since in SVG the embedded font ain't really a path
    // the outline has got to stay untransformed...
    qreal penWidth = p->pen().widthF();
    penWidth /= (pixelSize/m_unitsPerEm);
    QPen pen = p->pen();
    pen.setWidthF(penWidth);
    p->setPen(pen);

    QString::const_iterator itr = str.constBegin();
    for ( ; itr != str.constEnd(); ++itr) {
        QChar unicode = *itr;
        if (!m_glyphs.contains(*itr)) {
            unicode = 0;
            if (!m_glyphs.contains(unicode))
                continue;
        }
        p->drawPath(m_glyphs[unicode].m_path);
        p->translate(m_glyphs[unicode].m_horizAdvX, 0);
    }

    p->restore();
}

void QSvgFont::setFamilyName(const QString &name)
{
    m_familyName = name;
}

void QSvgFont::setUnitsPerEm(qreal upem)
{
    m_unitsPerEm = upem;
}
