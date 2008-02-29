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

#ifndef QOPENTYPE_P_H
#define QOPENTYPE_P_H

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

#include "qtextengine_p.h"

#ifndef QT_NO_OPENTYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#include "harfbuzz.h"
#include "ftglue.h"

enum { PositioningProperties = 0x80000000 };

class QShaperItem;

class QOpenType
{
public:
    QOpenType(QFontEngine *fe, FT_Face face);
    ~QOpenType();

    struct Features {
        uint tag;
        uint property;
    };

    bool supportsScript(unsigned int script) {
        Q_ASSERT(script < QUnicodeTables::ScriptCount);
        return supported_scripts[script];
    }
    void selectScript(QShaperItem *item, unsigned int script, const Features *features = 0);

    bool shape(QShaperItem *item, const unsigned int *properties = 0);
    bool positionAndAdd(QShaperItem *item, int availableGlyphs, bool doLogClusters = true);

    HB_GlyphItem glyphs() const { return hb_buffer->in_string; }
    int len() const { return hb_buffer->in_length; }
    void setProperty(int index, uint property) { hb_buffer->in_string[index].properties = property; }


private:
    bool checkScript(unsigned int script);
    QFontEngine *fontEngine;
    FT_Face face;
    HB_GDEF gdef;
    HB_GSUB gsub;
    HB_GPOS gpos;
    bool supported_scripts[QUnicodeTables::ScriptCount];
    FT_ULong current_script;
    bool positioned : 1;
    bool kerning_feature_selected : 1;
    HB_Buffer hb_buffer;
    QGlyphLayout::Attributes *tmpAttributes;
    unsigned int *tmpLogClusters;
    int length;
    int orig_nglyphs;
    int loadFlags;
};

#endif // QT_NO_OPENTYPE

#endif // QOPENTYPE_P_H
