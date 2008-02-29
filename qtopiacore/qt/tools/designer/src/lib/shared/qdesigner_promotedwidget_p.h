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

#ifndef QDESIGNER_PROMOTEDWIDGET_H
#define QDESIGNER_PROMOTEDWIDGET_H

#include <QtCore/QVariant>

#include <QtGui/QWidget>
#include <QtGui/QIcon>

#include "shared_global_p.h"
#include <QtDesigner/QtDesigner>
#include <QtDesigner/QExtensionFactory>

class QExtensionManager;

namespace qdesigner_internal {

class QDesignerPromotedWidget;

class QDESIGNER_SHARED_EXPORT PromotedWidgetPropertySheet: public QObject, public QDesignerPropertySheetExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension)
public:
    PromotedWidgetPropertySheet(QDesignerPromotedWidget *promoted, QExtensionManager *extension_manager, QObject *parent);
    virtual ~PromotedWidgetPropertySheet();

    virtual int count() const;

    virtual int indexOf(const QString &name) const;

    virtual QString propertyName(int index) const;
    virtual QString propertyGroup(int index) const;
    virtual void setPropertyGroup(int index, const QString &group);

    virtual bool hasReset(int index) const;
    virtual bool reset(int index);

    virtual bool isVisible(int index) const;
    virtual void setVisible(int index, bool b);

    virtual bool isAttribute(int index) const;
    virtual void setAttribute(int index, bool b);

    virtual QVariant property(int index) const;
    virtual void setProperty(int index, const QVariant &value);

    virtual bool isChanged(int index) const;
    virtual void setChanged(int index, bool changed);
private:
    QDesignerPromotedWidget *m_promoted;
    QDesignerPropertySheetExtension *m_sheet;
};

class QDESIGNER_SHARED_EXPORT PromotedWidgetPropertySheetFactory: public QExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(QAbstractExtensionFactory)
public:
    PromotedWidgetPropertySheetFactory(QExtensionManager *parent = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

class QDESIGNER_SHARED_EXPORT QDesignerPromotedWidget : public QWidget
{
    Q_OBJECT
public:
    QDesignerPromotedWidget(QDesignerWidgetDataBaseItemInterface *item, QWidget *parent = 0);
    ~QDesignerPromotedWidget();

    void setChildWidget(QWidget *widget);

    QWidget *child() const { return m_child; }
    QDesignerWidgetDataBaseItemInterface *item() const { return m_item; }
    const char *customClassName() { return m_custom_class_name.constData(); }

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

private:
    QDesignerWidgetDataBaseItemInterface *m_item;
    QByteArray m_custom_class_name;
    QWidget *m_child;
};

} // namespace qdesigner_internal

#endif // QDESIGNER_PROMOTEDWIDGET_H
