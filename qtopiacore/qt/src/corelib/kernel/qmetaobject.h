/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
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

#ifndef QMETAOBJECT_H
#define QMETAOBJECT_H

#include <QtCore/qobjectdefs.h>
#include <QtCore/qvariant.h>

QT_BEGIN_HEADER

QT_MODULE(Core)

template <typename T> class QList;

class Q_CORE_EXPORT QMetaMethod
{
public:
    inline QMetaMethod() : mobj(0),handle(0) {}

    const char *signature() const;
    const char *typeName() const;
    QList<QByteArray> parameterTypes() const;
    QList<QByteArray> parameterNames() const;
    const char *tag() const;
    enum Access { Private, Protected, Public };
    Access access() const;
    enum MethodType { Method, Signal, Slot };
    MethodType methodType() const;
    enum Attributes { Compatibility = 0x1, Cloned = 0x2, Scriptable = 0x4 };
    int attributes() const;

private:
    const QMetaObject *mobj;
    uint handle;
    friend struct QMetaObject;
};
Q_DECLARE_TYPEINFO(QMetaMethod, Q_MOVABLE_TYPE);

class Q_CORE_EXPORT QMetaEnum
{
public:
    inline QMetaEnum() : mobj(0),handle(0) {}

    const char *name() const;
    bool isFlag() const;

    int keyCount() const;
    const char *key(int index) const;
    int value(int index) const;

    const char *scope() const;

    int keyToValue(const char *key) const;
    const char* valueToKey(int value) const;
    int keysToValue(const char * keys) const;
    QByteArray valueToKeys(int value) const;


    inline bool isValid() const { return name() != 0; }
private:
    const QMetaObject *mobj;
    uint handle;
    friend struct QMetaObject;
};
Q_DECLARE_TYPEINFO(QMetaEnum, Q_MOVABLE_TYPE);

class Q_CORE_EXPORT QMetaProperty
{
public:
    QMetaProperty();

    const char *name() const;
    const char *typeName() const;
    QVariant::Type type() const;
    int userType() const;

    bool isReadable() const;
    bool isWritable() const;
    bool isResettable() const;
    bool isDesignable(const QObject *obj = 0) const;
    bool isScriptable(const QObject *obj = 0) const;
    bool isStored(const QObject *obj = 0) const;
    bool isEditable(const QObject *obj = 0) const;
    bool isUser(const QObject *obj = 0) const;

    bool isFlagType() const;
    bool isEnumType() const;
    QMetaEnum enumerator() const;

    QVariant read(const QObject *obj) const;
    bool write(QObject *obj, const QVariant &value) const;
    bool reset(QObject *obj) const;

    bool hasStdCppSet() const;
    inline bool isValid() const { return isReadable(); }

private:
    const QMetaObject *mobj;
    uint handle;
    int idx;
    QMetaEnum menum;
    friend struct QMetaObject;
};

class Q_CORE_EXPORT QMetaClassInfo
{
public:
    inline QMetaClassInfo() : mobj(0),handle(0) {}
    const char *name() const;
    const char *value() const;
private:
    const QMetaObject *mobj;
    uint handle;
    friend struct QMetaObject;
};
Q_DECLARE_TYPEINFO(QMetaClassInfo, Q_MOVABLE_TYPE);

QT_END_HEADER

#endif // QMETAOBJECT_H
