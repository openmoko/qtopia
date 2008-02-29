/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#define EQ 257
#define COLON 258
#define DOT 259
#define SEMICOLON 260
#define SPACE 261
#define HTAB 262
#define LINESEP 263
#define NEWLINE 264
#define BEGIN_VCARD 265
#define END_VCARD 266
#define BEGIN_VCAL 267
#define END_VCAL 268
#define BEGIN_VEVENT 269
#define END_VEVENT 270
#define BEGIN_VTODO 271
#define END_VTODO 272
#define ID 273
#define STRING 274
typedef union {
    char *str;
    VObject *vobj;
    } YYSTYPE;
extern YYSTYPE vcclval;
