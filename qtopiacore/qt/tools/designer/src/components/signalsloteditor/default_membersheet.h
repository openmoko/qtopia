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
