/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef DEFAULT_MEMBERSHEET_H
#define DEFAULT_MEMBERSHEET_H

#include <QtDesigner/membersheet.h>
#include <QtDesigner/default_extensionfactory.h>

class QDesignerMemberSheet: public QObject, public QDesignerMemberSheetExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerMemberSheetExtension)
public:
    QDesignerMemberSheet(QObject *object, QObject *parent = 0);
    virtual ~QDesignerMemberSheet();

    virtual int indexOf(const QString &name) const;

    virtual int count() const;
    virtual QString memberName(int index) const;

    virtual QString memberGroup(int index) const;
    virtual void setMemberGroup(int index, const QString &group);

    virtual bool isVisible(int index) const;
    virtual void setVisible(int index, bool b);

    virtual bool isSignal(int index) const;
    virtual bool isSlot(int index) const;
    virtual bool inheritedFromWidget(int index) const;

    virtual QString declaredInClass(int index) const;

    virtual QString signature(int index) const;
    virtual QList<QByteArray> parameterTypes(int index) const;
    virtual QList<QByteArray> parameterNames(int index) const;

protected:
    QObject *m_object;
    const QMetaObject *meta;

    class Info
    {
    public:
        QString group;
        uint visible: 1;

        inline Info()
            : visible(1) {}
    };

    QHash<int, Info> m_info;
};

class QDesignerMemberSheetFactory: public QExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(QAbstractExtensionFactory)
public:
    QDesignerMemberSheetFactory(QExtensionManager *parent = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

#endif // DEFAULT_MEMBERSHEET_H
