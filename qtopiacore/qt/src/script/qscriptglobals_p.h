/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtScript module of the Qt Toolkit.
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

#ifndef QSCRIPTGLOBALS_P_H
#define QSCRIPTGLOBALS_P_H

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

#include <QtCore/qglobal.h>

#ifndef QT_NO_SCRIPT

class QScriptValue;
class QScriptValueImpl;
class QScriptClassInfo;
class QScriptEngine;
class QScriptEnginePrivate;
class QScriptContext;
class QScriptContextPrivate;

typedef QScriptValueImpl (*QScriptInternalFunctionSignature)(QScriptContextPrivate *, QScriptEnginePrivate *, QScriptClassInfo *);
typedef QScriptValue (*QScriptFunctionSignature)(QScriptContext *, QScriptEngine *);

namespace QScript {

enum Type {
    ObjectBased         = 0x20000000,
    FunctionBased       = 0x40000000,

    UndefinedType       = 1,
    NullType            = 2,
    ReferenceType       = 3,

    // Integer based
    BooleanType         =  4,
    IntegerType         =  5,
    StringType          =  6,

    // Double based
    NumberType          =  7,

    // Pointer based
    PointerType         =  8,

    // Object data based
    ObjectType          =  9 | ObjectBased,
    FunctionType        = 10 | ObjectBased | FunctionBased,
    VariantType         = 11 | ObjectBased,
    QObjectType         = 12 | ObjectBased | FunctionBased,
    QMetaObjectType     = 13 | ObjectBased | FunctionBased,

    // Types used by the runtime
    ActivationType      = 100 | ObjectBased,

    CustomType          = 1000
};

} // namespace QScript

#endif // QT_NO_SCRIPT
#endif
