/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
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

/* This file is autogenerated from the Unicode 5.0 database. Do not edit */

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of internal files.  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//

#ifndef QUNICODETABLES_P_H
#define QUNICODETABLES_P_H

#include <QtCore/qchar.h>

namespace QUnicodeTables {
    struct Properties {
        ushort category : 8;
        ushort line_break_class : 8;
        ushort direction : 8;
        ushort combiningClass :8;
        ushort joining : 2;
        signed short digitValue : 6; /* 5 needed */
        ushort unicodeVersion : 4;
        ushort lowerCaseSpecial : 1;
        ushort upperCaseSpecial : 1;
        ushort titleCaseSpecial : 1;
        ushort caseFoldSpecial : 1; /* currently unused */
        signed short mirrorDiff : 16;
        signed short lowerCaseDiff : 16;
        signed short upperCaseDiff : 16;
        signed short titleCaseDiff : 16;
        signed short caseFoldDiff : 16;
    };
    Q_CORE_EXPORT const Properties * QT_FASTCALL properties(uint ucs4);
    Q_CORE_EXPORT const Properties * QT_FASTCALL properties(ushort ucs2);

    // See http://www.unicode.org/reports/tr24/tr24-5.html

    enum Script {
        Common,
        Greek,
        Cyrillic,
        Armenian,
        Hebrew,
        Arabic,
        Syriac,
        Thaana,
        Devanagari,
        Bengali,
        Gurmukhi,
        Gujarati,
        Oriya,
        Tamil,
        Telugu,
        Kannada,
        Malayalam,
        Sinhala,
        Thai,
        Lao,
        Tibetan,
        Myanmar,
        Georgian,
        Hangul,
        Ogham,
        Runic,
        Khmer,
        Inherited,
        ScriptCount = Inherited,
        Latin = Common,
        Ethiopic = Common,
        Cherokee = Common,
        CanadianAboriginal = Common,
        Mongolian = Common,
        Hiragana = Common,
        Katakana = Common,
        Bopomofo = Common,
        Han = Common,
        Yi = Common,
        OldItalic = Common,
        Gothic = Common,
        Deseret = Common,
        Tagalog = Common,
        Hanunoo = Common,
        Buhid = Common,
        Tagbanwa = Common,
        Limbu = Common,
        TaiLe = Common,
        LinearB = Common,
        Ugaritic = Common,
        Shavian = Common,
        Osmanya = Common,
        Cypriot = Common,
        Braille = Common,
        Buginese = Common,
        Coptic = Common,
        NewTaiLue = Common,
        Glagolitic = Common,
        Tifinagh = Common,
        SylotiNagri = Common,
        OldPersian = Common,
        Kharoshthi = Common,
        Balinese = Common,
        Cuneiform = Common,
        Phoenician = Common,
        PhagsPa = Common,
        Nko = Common
    };
    enum { ScriptSentinel = 32 };


    // see http://www.unicode.org/reports/tr14/tr14-19.html
    // we don't use the XX, AI and CB properties and map them to AL instead.
    // as we don't support any EBDIC based OS'es, NL is ignored and mapped to AL as well.
    enum LineBreakClass {
        LineBreak_OP, LineBreak_CL, LineBreak_QU, LineBreak_GL, LineBreak_NS,
        LineBreak_EX, LineBreak_SY, LineBreak_IS, LineBreak_PR, LineBreak_PO,
        LineBreak_NU, LineBreak_AL, LineBreak_ID, LineBreak_IN, LineBreak_HY,
        LineBreak_BA, LineBreak_BB, LineBreak_B2, LineBreak_ZW, LineBreak_CM,
        LineBreak_WJ, LineBreak_H2, LineBreak_H3, LineBreak_JL, LineBreak_JV,
        LineBreak_JT, LineBreak_SA, LineBreak_SG,
        LineBreak_SP, LineBreak_CR, LineBreak_LF, LineBreak_BK
    };


    Q_CORE_EXPORT QUnicodeTables::LineBreakClass QT_FASTCALL lineBreakClass(uint ucs4);
    inline int lineBreakClass(const QChar &ch) {
        return QUnicodeTables::lineBreakClass(ch.unicode());
    }

    Q_CORE_EXPORT int QT_FASTCALL script(uint ucs4);
    Q_CORE_EXPORT_INLINE int QT_FASTCALL script(const QChar &ch) {
        return script(ch.unicode());
    }


}
#endif
