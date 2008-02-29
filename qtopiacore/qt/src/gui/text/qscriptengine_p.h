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

#ifndef QSCRIPTENGINE_P_H
#define QSCRIPTENGINE_P_H

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

class QString;
struct QGlyphLayout;
struct QCharAttributes;

struct QShaperItem {
    int script;
    const QString *string;
    int from;
    int length;
    QFontEngine *font;
    QGlyphLayout *glyphs;
    int num_glyphs; // in: available glyphs out: glyphs used/needed
    unsigned short *log_clusters;
    int flags;
#if defined(Q_WS_X11) || defined (Q_WS_QWS)
    uint kerning_enabled : 1; // from QFont::kerning
    uint kerning_applied : 1; // out: kerning applied by shaper
#endif
    const QCharAttributes *charAttributes;
};

// return true if ok.
typedef bool (*ShapeFunction)(QShaperItem *item);
typedef void (*AttributeFunction)(int script, const QString &, int, int, QCharAttributes *);

struct q_scriptEngine {
    ShapeFunction shape;
    AttributeFunction charAttributes;
};

extern const q_scriptEngine qt_scriptEngines[];

struct QArabicProperties {
    unsigned char shape;
    unsigned char justification;
};
Q_DECLARE_TYPEINFO(QArabicProperties, Q_PRIMITIVE_TYPE);

enum QArabicShape {
    XIsolated,
    XFinal,
    XInitial,
    XMedial,
    // intermediate state
    XCausing
};

void qt_getArabicProperties(const unsigned short *chars, int len, QArabicProperties *properties);

#endif // QSCRIPTENGINE_P_H
