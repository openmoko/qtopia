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

#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include "propertyeditor_global.h"
#include "qpropertyeditor.h"

#include <QtDesigner/QtDesigner>

#include <QtCore/QPointer>

class DomProperty;
class QDesignerPropertySheetExtension;

namespace qdesigner_internal {
    class MetaDataBaseItem;

class QT_PROPERTYEDITOR_EXPORT PropertyEditor: public QDesignerPropertyEditorInterface
{
    Q_OBJECT
public:
    PropertyEditor(QDesignerFormEditorInterface *core, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~PropertyEditor();

    virtual QDesignerFormEditorInterface *core() const;

    virtual bool isReadOnly() const;
    virtual void setReadOnly(bool readOnly);
    virtual void setPropertyValue(const QString &name, const QVariant &value, bool changed = true);
    virtual void setObject(QObject *object);

    virtual QObject *object() const
    { return m_object; }

    virtual QString currentPropertyName() const;

private slots:
    void firePropertyChanged(IProperty *property);
    void resetProperty(const QString &prop_name);

private:
    IProperty *propertyByName(IProperty *p, const QString &name);
    void clearDirty(IProperty *p);
    void createPropertySheet(PropertyCollection *root, QObject *object);
    static IProperty *createSpecialProperty(const QVariant &value, const QString &name);

private:
    MetaDataBaseItem *metaDataBaseItem() const;
                
    QDesignerFormEditorInterface *m_core;
    QPropertyEditor *m_editor;
    IPropertyGroup *m_properties;
    QDesignerPropertySheetExtension *m_prop_sheet;
    QPointer<QObject> m_object;
};

}  // namespace qdesigner_internal

#endif // PROPERTYEDITOR_H
