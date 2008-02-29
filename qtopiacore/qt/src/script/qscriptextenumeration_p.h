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

#ifndef QSCRIPTEXTENUMERATION_P_H
#define QSCRIPTEXTENUMERATION_P_H

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

#include "qscriptecmacore_p.h"

#ifndef QT_NO_SCRIPT

#include <QtCore/QSet>

namespace QScript { namespace Ext {

class EnumerationClassData: public QScriptClassData
{
    QScriptClassInfo *m_classInfo;

public:
    EnumerationClassData(QScriptClassInfo *classInfo);
    virtual ~EnumerationClassData();

    inline QScriptClassInfo *classInfo() const
        { return m_classInfo; }

    virtual void mark(const QScriptValueImpl &object, int generation);
};

class Enumeration: public QScript::Ecma::Core
{
public:
    Enumeration(QScriptEnginePrivate *engine);
    virtual ~Enumeration();

    inline QScriptClassInfo *classInfo() const
        { return m_classInfo; }

    virtual void execute(QScriptContextPrivate *context);

    class Instance: public QScriptObjectData {
    public:
        Instance() {}
        virtual ~Instance() {}

        static Instance *get(const QScriptValueImpl &object,
                             QScriptClassInfo *klass);

        void toFirst();
        void hasNext(QScriptContextPrivate *context, QScriptValueImpl *result);
        void next(QScriptContextPrivate *context, QScriptValueImpl *result);

    public: // attributes
        QScriptValueImpl object;
        QScriptValueImpl value;
        int index;
    };

    void newEnumeration(QScriptValueImpl *result, const QScriptValueImpl &value);

    inline Instance *get(const QScriptValueImpl &object) const
    {
        return Instance::get(object, m_classInfo);
    }

protected:
    static QScriptValueImpl method_toFirst(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValueImpl method_hasNext(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValueImpl method_next(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                        QScriptClassInfo *classInfo);

private:
    QScriptClassInfo *m_classInfo;
};

} } // namespace QScript::Ext

#endif // QT_NO_SCRIPT
#endif
