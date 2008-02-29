/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QFONTSUBSET_P_H
#define QFONTSUBSET_P_H

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

#include "private/qfontengine_p.h"

#ifndef QT_NO_PRINTER

class QFontSubset
{
public:
    QFontSubset(QFontEngine *fe, int obj_id = 0)
        : object_id(obj_id), noEmbed(false), fontEngine(fe), downloaded_glyphs(0), standard_font(false)
        { fontEngine->ref.ref(); addGlyph(0); }
    ~QFontSubset() {
        if (!fontEngine->ref.deref())
            delete fontEngine;
    }

    QByteArray toTruetype() const;
    QByteArray toType1() const;
    QByteArray type1AddedGlyphs() const;
    QByteArray widthArray() const;
    QByteArray createToUnicodeMap() const;
    QVector<int> getReverseMap() const;
    QByteArray glyphName(unsigned int glyph, const QVector<int> reverseMap) const;

    static QByteArray glyphName(unsigned short unicode, bool symbol);

    int addGlyph(int index);
    const int object_id;
    bool noEmbed;
    QFontEngine *fontEngine;
    QList<int> glyph_indices;
    mutable int downloaded_glyphs;
    mutable bool standard_font;
    int nGlyphs() const { return glyph_indices.size(); }
    mutable QFixed emSquare;
    mutable QVector<QFixed> widths;
};

#endif
#endif
