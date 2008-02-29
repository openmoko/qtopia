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

#ifndef QPROPERTYEDITOR_MODEL_P_H
#define QPROPERTYEDITOR_MODEL_P_H

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

#include "qpropertyeditor_items_p.h"
#include <qabstractitemmodel.h>

namespace qdesigner_internal {

class QPropertyEditorModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    QPropertyEditorModel(QObject *parent = 0);
    ~QPropertyEditorModel();

    inline IProperty *initialInput() const
    { return m_initialInput; }

    inline QModelIndex indexOf(IProperty *property, int column = 0) const
    {
        Q_ASSERT(property);
        if (property == m_initialInput)
            return createIndex(0, column, m_initialInput);

        IProperty *parent = parentOf(property);
        if (!parent || parent->kind() != IProperty::Property_Group)
            return QModelIndex();

        int row = static_cast<IPropertyGroup*>(parent)->indexOf(property);
        return createIndex(row, column, property);
    }

    inline IProperty *privateData(const QModelIndex &index) const
    { return static_cast<IProperty*>(index.internalPointer()); }

    Qt::ItemFlags flags(const QModelIndex &index) const;

signals:
    void propertyChanged(IProperty *property);
    void resetProperty(const QString &name);

public slots:
    void setInitialInput(IProperty *initialInput);
    void refresh(IProperty *property);

public:
//
// QAbstractItemModel interface
//
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;

    virtual QModelIndex parent(const QModelIndex &index) const;

    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual bool hasChildren(const QModelIndex &parent) const
    { return rowCount(parent) > 0; }

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

    virtual bool isEditable(const QModelIndex &index) const;
    virtual QModelIndex buddy(const QModelIndex &index) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

protected:
    QString columnText(int column) const;

    inline IProperty *childAt(IProperty *parent, int pos) const
    {
        if (parent && parent->kind() == IProperty::Property_Group)
            return static_cast<IPropertyGroup*>(parent)->propertyAt(pos);

        return 0;
    }

    inline IProperty *parentOf(IProperty *property) const
    { return property ? property->parent() : 0; }

    static IPropertyGroup *toPropertyGroup(IProperty *property)
    {
        if (!property || property->kind() != IProperty::Property_Group)
            return 0;

        return static_cast<IPropertyGroup*>(property);
    }

private:
    IProperty *m_initialInput;
};

}  // namespace qdesigner_internal

#endif // QPROPERTYEDITOR_MODEL_P_H
