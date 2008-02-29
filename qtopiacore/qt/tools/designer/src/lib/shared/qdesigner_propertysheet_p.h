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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDESIGNER_PROPERTYSHEET_H
#define QDESIGNER_PROPERTYSHEET_H

#include "shared_global_p.h"
#include <QtDesigner/propertysheet.h>
#include <QtDesigner/default_extensionfactory.h>
#include <QtCore/QVariant>
#include <QtCore/QPair>

class QDESIGNER_SHARED_EXPORT QDesignerPropertySheet: public QObject, public QDesignerPropertySheetExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension)
public:
    QDesignerPropertySheet(QObject *object, QObject *parent = 0);
    virtual ~QDesignerPropertySheet();

    virtual int indexOf(const QString &name) const;

    virtual int count() const;
    virtual QString propertyName(int index) const;

    virtual QString propertyGroup(int index) const;
    virtual void setPropertyGroup(int index, const QString &group);

    virtual bool hasReset(int index) const;
    virtual bool reset(int index);

    virtual bool isAttribute(int index) const;
    virtual void setAttribute(int index, bool b);

    virtual bool isVisible(int index) const;
    virtual void setVisible(int index, bool b);

    virtual QVariant property(int index) const;
    virtual void setProperty(int index, const QVariant &value);

    virtual bool isChanged(int index) const;
    virtual void setChanged(int index, bool changed);

    void createFakeProperty(const QString &propertyName, const QVariant &value = QVariant());

protected:
    bool isAdditionalProperty(int index) const;
    bool isFakeProperty(int index) const;
    QVariant resolvePropertyValue(const QVariant &value) const;
    QVariant metaProperty(int index) const;
    void setFakeProperty(int index, const QVariant &value);

    bool isFakeLayoutProperty(int index) const;

protected:
    QObject *m_object;
    const QMetaObject *meta;

    class Info
    {
    public:
        QString group;
        uint changed: 1;
        uint visible: 1;
        uint attribute: 1;
        uint reset: 1;

        inline Info()
            : changed(0),
              visible(1),
              attribute(0),
              reset(1)
        {}
    };

    QHash<int, Info> m_info;
    QHash<int, QVariant> m_fakeProperties;
    QHash<int, QVariant> m_addProperties;
    QHash<QString, int> m_addIndex;
};

class QDESIGNER_SHARED_EXPORT QDesignerPropertySheetFactory: public QExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(QAbstractExtensionFactory)
public:
    QDesignerPropertySheetFactory(QExtensionManager *parent = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

#endif // QDESIGNER_PROPERTYSHEET_H
