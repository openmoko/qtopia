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

#ifndef QTRANSLATOR_P_H
#define QTRANSLATOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qfontencodings_x11.cpp and qfont_x11.cpp.  This header file may
// change from version to version without notice, or even be removed.
//
// We mean it.
//

enum {
    EQ          = 0x01,
    LT          = 0x02,
    LEQ         = 0x03,
    BETWEEN     = 0x04,

    NOT         = 0x08,
    MOD_10      = 0x10,
    MOD_100     = 0x20,

    AND         = 0xFD,
    OR          = 0xFE,
    NEWRULE     = 0xFF,

    OP_MASK     = 0x07,

    NEQ         = NOT | EQ,
    GT          = NOT | LEQ,
    GEQ         = NOT | LT,
    NOT_BETWEEN = NOT | BETWEEN
};

#endif
