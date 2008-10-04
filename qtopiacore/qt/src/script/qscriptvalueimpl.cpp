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

#include "qscriptvalueimpl_p.h"

#ifndef QT_NO_SCRIPT

#include "qscriptengine_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

QT_BEGIN_NAMESPACE

static void dfs(QScriptObject *instance, QHash<QScriptObject*, int> &dfn, int n)
{
    bool found = dfn.contains(instance);
    dfn[instance] = n;

    if (found)
        return;

    if (instance->m_prototype.isObject())
        dfs (instance->m_prototype.m_object_value, dfn, n + 1);

    if (instance->m_scope.isObject())
        dfs (instance->m_scope.m_object_value, dfn, n + 1);
}


static bool checkCycle(QScriptObject *instance, const QHash<QScriptObject*, int> &dfn)
{
    int n = dfn.value(instance);

    if (instance->m_prototype.isObject()) {
        if (n >= dfn.value(instance->m_prototype.m_object_value))
            return true;
    }

    if (instance->m_scope.isObject()) {
        if (n >= dfn.value(instance->m_scope.m_object_value))
            return true;
    }

    return false;
}

bool QScriptValueImpl::detectedCycle() const
{
    QHash<QScriptObject*, int> dfn;
    dfs(m_object_value, dfn, 0);
    return checkCycle(m_object_value, dfn);
}

bool QScriptValueImpl::instanceOf(const QScriptValueImpl &value) const
{
    if (! isObject() || ! value.isObject())
        return false;
    if (value.isFunction()) {
        QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
        QScriptValueImpl proto = value.property(eng_p->idTable()->id_prototype);
        if (proto.isObject())
            return instanceOf_helper(proto);
    }
    return instanceOf_helper(value);
}

bool QScriptValueImpl::instanceOf_helper(const QScriptValueImpl &value) const
{
    Q_ASSERT(isObject());
    Q_ASSERT(value.isObject());

    QScriptObject *instance = m_object_value;
    QScriptObject *target = value.m_object_value;

    if (instance == target)
        return false;

    while (instance != 0) {
        if (instance == target)
            return true;

        const QScriptValueImpl &proto = instance->m_prototype;

        if (! proto.isObject())
            break;

        instance = proto.m_object_value;
    }

    return false;
}

bool QScriptValueImpl::resolve_helper(QScriptNameIdImpl *nameId, QScript::Member *member,
                                      QScriptValueImpl *object, QScriptValue::ResolveFlags mode) const
{
    QScriptObject *object_data = m_object_value;

    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());

    if (nameId == eng_p->idTable()->id___proto__) {
        member->native(nameId, /*id=*/0, QScriptValue::Undeletable);
        *object = *this;
        return true;
    }

    // If not found anywhere else, search in the extra members.
    if (QScriptClassData *odata = classInfo()->data().data()) {
        *object = *this;

        if (odata->resolve(*this, nameId, member, object))
            return true;
    }

    if (mode & QScriptValue::ResolvePrototype) {
        // For values and other non object based types, search in class's prototype
        const QScriptValueImpl &proto = object_data->m_prototype;

        if (proto.isObject()
            && proto.resolve(nameId, member, object, mode)) {
            return true;
        }
    }

    if ((mode & QScriptValue::ResolveScope) && object_data->m_scope.isValid())
        return object_data->m_scope.resolve(nameId, member, object, mode);

    return false;
}

void QScriptValueImpl::setProperty(QScriptNameIdImpl *nameId,
                                   const QScriptValueImpl &value,
                                   const QScriptValue::PropertyFlags &flags)
{
    if (!isObject())
        return;

    QScriptValueImpl base;
    QScript::Member member;

    QScriptValue::ResolveFlags mode = QScriptValue::ResolveLocal;
    // if we are not setting a setter or getter, look in prototype too
    if (!(flags & (QScriptValue::PropertyGetter | QScriptValue::PropertySetter)))
        mode |= QScriptValue::ResolvePrototype;

    if (resolve(nameId, &member, &base, mode)) {
        // we resolved an existing property with that name
        if (flags & (QScriptValue::PropertyGetter | QScriptValue::PropertySetter)) {
            // setting the getter or setter of a property in this object
            if (member.isNativeProperty()) {
                if (value.isValid()) {
                    qWarning("QScriptValue::setProperty() failed: "
                             "cannot set getter or setter of native property `%s'",
                             qPrintable(nameId->s));
                }
                return;
            }
            if (member.isSetter()) {
                // the property we resolved is a setter
                if (!(flags & QScriptValue::PropertySetter) && !member.isGetter()) {
                    // find the getter, if not, create one
                    if (!m_object_value->findGetter(&member)) {
                        if (!value.isValid())
                            return; // don't create property for invalid value
                        createMember(nameId, &member, flags);
                    }
                }
            } else if (member.isGetter()) {
                // the property we resolved is a getter
                if (!(flags & QScriptValue::PropertyGetter)) {
                    // find the setter, if not, create one
                    if (!m_object_value->findSetter(&member)) {
                        if (!value.isValid())
                            return; // don't create property for invalid value
                        createMember(nameId, &member, flags);
                    }
                }
            } else {
                // the property is a normal property -- change the flags
                uint newFlags = flags & ~QScript::Member::InternalRange;
                newFlags |= QScript::Member::ObjectProperty;
                member.resetFlags(newFlags);
                base.m_object_value->m_members[member.id()].resetFlags(newFlags);
            }
            Q_ASSERT(member.isValid());
            if (!value.isValid()) {
                // remove the property
                removeMember(member);
                return;
            }
        } else {
            // setting the value
            if (member.isGetterOrSetter()) {
                // call the setter
                QScriptValueImpl setter;
                if (member.isObjectProperty() && !member.isSetter()) {
                    if (!base.m_object_value->findSetter(&member)) {
                        qWarning("QScriptValue::setProperty() failed: "
                                 "property '%s' has a getter but no setter",
                                 qPrintable(nameId->s));
                        return;
                    }
                }
                base.get(member, &setter);
                setter.call(*this, QScriptValueImplList() << value);
                return;
            } else {
                if (base.m_object_value != m_object_value) {
                    if (!value.isValid())
                        return; // don't create property for invalid value
                    createMember(nameId, &member, flags);
                    base = *this;
                } else {
                    if (!value.isValid()) {
                        // remove the property
                        removeMember(member);
                        return;
                    }
                }
                if (flags != QScriptValue::KeepExistingFlags) {
                    // change flags
                    if (member.isNativeProperty()) {
                        qWarning("QScriptValue::setProperty(%s): "
                                 "cannot change flags of a native property",
                                 qPrintable(nameId->s));
                    } else {
                        uint newFlags = member.flags() & QScript::Member::InternalRange;
                        newFlags |= flags & ~QScript::Member::InternalRange;
                        base.m_object_value->m_members[member.id()].resetFlags(newFlags);
                    }
                }
            }
        }
    } else {
        // property does not exist
        if (!value.isValid())
            return; // don't create property for invalid value
        createMember(nameId, &member, flags & ~QScript::Member::InternalRange);
        base = *this;
    }

    base.put(member, value);
}

QVariant QScriptValueImpl::toVariant() const
{
    if (!isValid())
        return QVariant();
    switch (m_type->type()) {

    case QScript::UndefinedType:
    case QScript::NullType:
    case QScript::PointerType:
    case QScript::ReferenceType:
        break;

    case QScript::BooleanType:
        return QVariant(m_bool_value);

    case QScript::IntegerType:
        return QVariant(m_int_value);

    case QScript::NumberType:
        return QVariant(m_number_value);

    case QScript::StringType:
        return QVariant(m_string_value->s);

    case QScript::ObjectType:
        if (isDate())
            return QVariant(toDateTime());

#ifndef QT_NO_REGEXP
        if (isRegExp())
            return QVariant(toRegExp());
#endif
        if (isVariant())
            return variantValue();

#ifndef QT_NO_QOBJECT
        if (isQObject())        
            return qVariantFromValue(toQObject());
#endif

        QScriptValue v = toPrimitive();
        if (!v.isObject())
            return v.toVariant();
        break;
    } // switch
    return QVariant();
}

QDebug &operator<<(QDebug &d, const QScriptValueImpl &object)
{
    d.nospace() << "QScriptValue(";

    if (!object.isValid()) {
        d.nospace() << "Invalid)";
        return d;
    }

    switch (object.type()) {
    case QScript::BooleanType:
        d.nospace() << "bool=" << object.toBoolean();
        break;

    case QScript::IntegerType:
        d.nospace() << "int=" << object.toInt32();
        break;

    case QScript::NumberType:
        d.nospace() << "qsreal=" << object.toNumber();
        break;

    case QScript::StringType:
        d.nospace() << "string=" << object.toString();
        break;

    case QScript::ReferenceType:
        d.nospace() << "reference";
        break;

    case QScript::NullType:
        d.nospace() << "null";
        break;

    case QScript::UndefinedType:
        d.nospace() << "undefined";
        break;

    case QScript::PointerType:
        d.nospace() << "pointer";
        break;

    case QScript::ObjectType:
        d.nospace() << object.classInfo()->name() << ",{";
        QScriptObject *od = object.objectValue();
        for (int i=0; i<od->memberCount(); ++i) {
            if (i != 0)
                d << ",";

            QScript::Member m;
            od->member(i, &m);

            if (m.isValid() && m.isObjectProperty()) {
                d << QScriptEnginePrivate::get(object.engine())->toString(m.nameId());
                QScriptValueImpl o;
                od->get(m, &o);
                d.nospace() << QLatin1String(":")
                            << (o.classInfo()
                                ? o.classInfo()->name()
                                : QLatin1String("?"));
            }
        }

        d.nospace() << "} scope={";
        QScriptValueImpl scope = object.scope();
        while (scope.isValid()) {
            Q_ASSERT(scope.isObject());
            d.nospace() << " " << scope.objectValue();
            scope = scope.scope();
        }
        d.nospace() << "}";
        break;
    }

    d << ")";
    return d;
}

void QScriptValueImpl::destroyObjectData()
{
    Q_ASSERT(isObject());
    m_object_value->finalizeData(engine());
}

bool QScriptValueImpl::isMarked(int generation) const
{
    if (isString())
        return (m_string_value->used != 0);
    else if (isObject()) {
        QScript::GCBlock *block = QScript::GCBlock::get(m_object_value);
        return (block->generation == generation);
    }
    return false;
}

QT_END_NAMESPACE

#endif // QT_NO_SCRIPT
