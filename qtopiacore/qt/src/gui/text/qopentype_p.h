/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
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
    bool positioned;
    bool kerning_feature_selected;
    bool glyphs_substituted;
    bool has_features;
    HB_Buffer hb_buffer;
    int allocated;
    QGlyphLayout::Attributes *tmpAttributes;
    unsigned int *tmpLogClusters;
    int length;
    int orig_nglyphs;
    int loadFlags;
};

#endif // QT_NO_OPENTYPE

#endif // QOPENTYPE_P_H
