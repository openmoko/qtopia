/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtScript module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QSCRIPTVALUEIMPLFWD_P_H
#define QSCRIPTVALUEIMPLFWD_P_H

#include "qscripttypeinfo_p.h"

#ifndef QT_NO_SCRIPT

#include "qscriptvalue.h"

#include <QtCore/qstring.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

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

class QScriptValueImpl;
typedef QList<QScriptValueImpl> QScriptValueImplList;

class QScriptClassInfo;
class QScriptObject;
class QScriptObjectData;
class QScriptNameIdImpl;
class QScriptFunction;
class QScriptEnginePrivate;

namespace QScript
{
    class Member;
};

class QScriptValueImpl
{
public:
    enum TypeHint {
        NoTypeHint,
        NumberTypeHint,
        StringTypeHint
    };

    inline QScriptValueImpl();
    inline QScriptValueImpl(QScriptEnginePrivate *engine, QScriptValue::SpecialValue val);
    inline QScriptValueImpl(QScriptEnginePrivate *engine, bool val);
    inline QScriptValueImpl(QScriptEnginePrivate *engine, int val);
    inline QScriptValueImpl(QScriptEnginePrivate *engine, uint val);
    inline QScriptValueImpl(QScriptEnginePrivate *engine, qsreal val);
    inline QScriptValueImpl(QScriptEnginePrivate *engine, const QString &val);
    inline QScriptValueImpl(QScriptEnginePrivate *engine, QScriptNameIdImpl *val);

    inline QScript::Type type() const;
    inline QScriptEngine *engine() const;
    inline QScriptTypeInfo *typeInfo() const;
    inline void setTypeInfo(QScriptTypeInfo *type);
    inline QScriptClassInfo *classInfo() const;
    inline void setClassInfo(QScriptClassInfo *cls);
    inline QScriptNameIdImpl *stringValue() const;
    inline QScriptObject *objectValue() const;
    inline void incr();
    inline void decr();

    inline void invalidate();
    inline bool isValid() const;
    inline bool isBoolean() const;
    inline bool isNumber() const;
    inline bool isString() const;
    inline bool isFunction() const;
    inline bool isObject() const;
    inline bool isUndefined() const;
    inline bool isNull() const;
    inline bool isVariant() const;
    inline bool isQObject() const;
    inline bool isQMetaObject() const;
    inline bool isReference() const;

    inline bool isError() const;
    inline bool isArray() const;
    inline bool isDate() const;
    inline bool isRegExp() const;

    inline QString toString() const;
    inline qsreal toNumber() const;
    inline bool toBoolean() const;
    inline qsreal toInteger() const;
    inline qint32 toInt32() const;
    inline quint32 toUInt32() const;
    inline quint16 toUInt16() const;
    QVariant toVariant() const;
    inline QObject *toQObject() const;
    inline const QMetaObject *toQMetaObject() const;
    inline QScriptValueImpl toObject() const;
    inline QDateTime toDateTime() const;
#ifndef QT_NO_REGEXP
    inline QRegExp toRegExp() const;
#endif
    inline QScriptValueImpl toPrimitive(TypeHint hint = NoTypeHint) const;

    inline QVariant &variantValue() const;
    inline void setVariantValue(const QVariant &v);

    bool instanceOf(const QScriptValueImpl &value) const;
    bool instanceOf_helper(const QScriptValueImpl &value) const;

    inline QScriptValueImpl prototype() const;
    inline void setPrototype(const QScriptValueImpl &prototype);

    inline QScriptValueImpl property(QScriptNameIdImpl *nameId,
                                     const QScriptValue::ResolveFlags &mode = QScriptValue::ResolvePrototype) const;
    void setProperty(QScriptNameIdImpl *nameId, const QScriptValueImpl &value,
                     const QScriptValue::PropertyFlags &flags = QScriptValue::KeepExistingFlags);

    inline QScriptValueImpl property(const QString &name,
                              const QScriptValue::ResolveFlags &mode = QScriptValue::ResolvePrototype) const;
    inline void setProperty(const QString &name, const QScriptValueImpl &value,
                     const QScriptValue::PropertyFlags &flags = QScriptValue::KeepExistingFlags);

    inline QScriptValueImpl property(quint32 arrayIndex,
                              const QScriptValue::ResolveFlags &mode = QScriptValue::ResolvePrototype) const;
    inline void setProperty(quint32 arrayIndex, const QScriptValueImpl &value,
                     const QScriptValue::PropertyFlags &flags = QScriptValue::KeepExistingFlags);

    inline QScriptValue::PropertyFlags propertyFlags(const QString &name,
                                                     const QScriptValue::ResolveFlags &mode = QScriptValue::ResolvePrototype) const;
    inline QScriptValue::PropertyFlags propertyFlags(QScriptNameIdImpl *nameId,
                                                     const QScriptValue::ResolveFlags &mode = QScriptValue::ResolvePrototype) const;

    inline bool deleteProperty(QScriptNameIdImpl *nameId,
                               const QScriptValue::ResolveFlags &mode = QScriptValue::ResolveLocal);

    inline QScriptValueImpl call(const QScriptValueImpl &thisObject = QScriptValueImpl(),
                                 const QScriptValueImplList &args = QScriptValueImplList());
    inline QScriptValueImpl call(const QScriptValueImpl &thisObject,
                                 const QScriptValueImpl &arguments);
    inline QScriptValueImpl construct(const QScriptValueImplList &args = QScriptValueImplList());
    inline QScriptValueImpl construct(const QScriptValueImpl &arguments);

    inline void mark(int) const;
    bool isMarked(int) const;

    inline operator QScriptValue() const;

    inline QScriptValueImpl internalValue() const;
    inline void setInternalValue(const QScriptValueImpl &internalValue);

    inline void setQObjectValue(QObject *object);

    inline QScriptObjectData *objectData() const;
    inline void setObjectData(QScriptObjectData *data);
    void destroyObjectData();

    inline void createMember(QScriptNameIdImpl *nameId,
                      QScript::Member *member, uint flags); // ### remove me
    inline int memberCount() const;
    inline void member(int index, QScript::Member *member) const;

    inline bool resolve(QScriptNameIdImpl *nameId, QScript::Member *member,
                 QScriptValueImpl *object, QScriptValue::ResolveFlags mode) const;
    bool resolve_helper(QScriptNameIdImpl *nameId, QScript::Member *member,
                        QScriptValueImpl *object, QScriptValue::ResolveFlags mode) const;
    inline void get(const QScript::Member &member, QScriptValueImpl *out) const;
    inline void get_helper(const QScript::Member &member, QScriptValueImpl *out) const;
    inline void get(QScriptNameIdImpl *nameId, QScriptValueImpl *out);
    inline void put(const QScript::Member &member, const QScriptValueImpl &value);
    inline void removeMember(const QScript::Member &member);

    inline QScriptValueImpl scope() const;
    inline void setScope(const QScriptValueImpl &scope);

    inline QScriptFunction *toFunction() const;

    inline bool lessThan(const QScriptValueImpl &other) const;
    inline bool equals(const QScriptValueImpl &other) const;
    inline bool strictlyEquals(const QScriptValueImpl &other) const;

    bool detectedCycle() const;

    union {
        bool m_bool_value;
        int m_int_value;
        qsreal m_number_value;
        void *m_ptr_value;
        QScriptObject *m_object_value;
        QScriptNameIdImpl *m_string_value;
    };
    QScriptTypeInfo *m_type;
};

QT_END_NAMESPACE

#endif // QT_NO_SCRIPT

#endif // QSCRIPTVALUEIMPLFWD_P_H
