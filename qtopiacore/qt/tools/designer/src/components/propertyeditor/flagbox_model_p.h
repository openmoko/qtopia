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

#ifndef FLAGBOX_MODEL_P_H
#define FLAGBOX_MODEL_P_H

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

#include "propertyeditor_global.h"
#include <QtCore/QAbstractItemModel>
#include <QtCore/QList>

namespace qdesigner_internal {

class QT_PROPERTYEDITOR_EXPORT FlagBoxModelItem
{
public:
    FlagBoxModelItem(const QString &name, unsigned int value, bool checked = false)
        : m_name(name), m_value(value), m_checked(checked) {}

    inline unsigned int value() const { return m_value; }

    inline QString name() const { return m_name; }
    inline void setName(const QString &name) { m_name = name; }

    inline bool isChecked() const { return m_checked; }
    inline void setChecked(bool checked) { m_checked = checked; }

private:
    QString m_name;
    unsigned int m_value;
    bool m_checked;
};

class QT_PROPERTYEDITOR_EXPORT FlagBoxModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    FlagBoxModel(QObject *parent = 0);
    virtual ~FlagBoxModel();

    inline FlagBoxModelItem itemAt(int index) const
    { return m_items.at(index); }

    inline FlagBoxModelItem &item(int index)
    { return m_items[index]; }

    inline QList<FlagBoxModelItem> items() const { return m_items; }
    void setItems(const QList<FlagBoxModelItem> &items);

    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual bool hasChildren(const QModelIndex &parent) const
    { return rowCount(parent) > 0; }

    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    static int bitcount(int mask);

private:
    QList<FlagBoxModelItem> m_items;
};

}  // namespace qdesigner_internal

#endif // FLAGBOX_MODEL_P_H
