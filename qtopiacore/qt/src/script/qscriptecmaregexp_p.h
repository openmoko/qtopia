/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
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
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSCRIPTECMAREGEXP_P_H
#define QSCRIPTECMAREGEXP_P_H

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

#include <QtCore/QRegExp>

#ifndef QT_NO_SCRIPT

#include "qscriptecmacore_p.h"

namespace QScript { namespace Ecma {

class RegExp: public Core
{
public:
    RegExp(QScriptEnginePrivate *engine);
    virtual ~RegExp();

    inline QScriptClassInfo *classInfo() const
        { return m_classInfo; }

    virtual void execute(QScriptContextPrivate *context);

    class Instance: public QScriptObjectData {
    public:
        Instance() {}
        virtual ~Instance() {}

        static Instance *get(const QScriptValueImpl &object,
                             QScriptClassInfo *klass);

    public: // attributes
#ifndef QT_NO_REGEXP
        QRegExp value;
#else
        QString pattern;
#endif
        QString flags;
    };

    inline Instance *get(const QScriptValueImpl &object) const
        { return Instance::get(object, m_classInfo); }

    void newRegExp(QScriptValueImpl *result, const QString &pattern,
                   const QString &flags);
#ifndef QT_NO_REGEXP
    void newRegExp(QScriptValueImpl *result, const QRegExp &rx,
                   const QString &flags = QString());
    QRegExp toRegExp(const QScriptValueImpl &value) const;
#endif

protected:
    static QScriptValueImpl method_exec(QScriptContextPrivate *context,
                                        QScriptEnginePrivate *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValueImpl method_test(QScriptContextPrivate *context,
                                        QScriptEnginePrivate *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValueImpl method_toString(QScriptContextPrivate *context,
                                            QScriptEnginePrivate *eng,
                                            QScriptClassInfo *classInfo);

private:
#ifndef QT_NO_REGEXP
    void newRegExp_helper(QScriptValueImpl *result, const QRegExp &rx,
                          const QString &flags);
#endif

    QScriptClassInfo *m_classInfo;
};

} } // namespace QScript::Ecma

#endif // QT_NO_SCRIPT
#endif // QSCRIPTECMAREGEXP_P_H
