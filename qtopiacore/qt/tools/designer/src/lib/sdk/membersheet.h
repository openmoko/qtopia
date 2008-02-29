/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MEMBERSHEET_H
#define MEMBERSHEET_H

#include <QtDesigner/extension.h>

#include <QtCore/QList>
#include <QtCore/QByteArray>

QT_BEGIN_HEADER

class QDesignerMemberSheetExtension
{
public:
    virtual ~QDesignerMemberSheetExtension() {}

    virtual int count() const = 0;

    virtual int indexOf(const QString &name) const = 0;

    virtual QString memberName(int index) const = 0;
    virtual QString memberGroup(int index) const = 0;
    virtual void setMemberGroup(int index, const QString &group) = 0;

    virtual bool isVisible(int index) const = 0;
    virtual void setVisible(int index, bool b) = 0;

    virtual bool isSignal(int index) const = 0;
    virtual bool isSlot(int index) const = 0;
    virtual bool inheritedFromWidget(int index) const = 0;

    virtual QString declaredInClass(int index) const = 0;

    virtual QString signature(int index) const = 0;
    virtual QList<QByteArray> parameterTypes(int index) const = 0;
    virtual QList<QByteArray> parameterNames(int index) const = 0;
};
Q_DECLARE_EXTENSION_INTERFACE(QDesignerMemberSheetExtension, "com.trolltech.Qt.Designer.MemberSheet")

QT_END_HEADER

#endif // MEMBERSHEET_H
