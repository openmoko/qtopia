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

#ifndef METADATABASE_H
#define METADATABASE_H

#include "shared_global_p.h"

#include <QtDesigner/QDesignerMetaDataBaseInterface>

#include <QtCore/QHash>
#include <QtGui/QCursor>

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT MetaDataBaseItem: public QDesignerMetaDataBaseItemInterface
{
public:
    MetaDataBaseItem(QObject *object);
    virtual ~MetaDataBaseItem();

    virtual QString name() const;
    virtual void setName(const QString &name);

    virtual QList<QWidget*> tabOrder() const;
    virtual void setTabOrder(const QList<QWidget*> &tabOrder);

    virtual bool enabled() const;
    virtual void setEnabled(bool b);

    QString propertyComment(const QString &name) const;
    void setPropertyComment(const QString &name, const QString &comment);

    QHash<QString, QString> comments() const { return m_comments; }

private:
    QObject *m_object;
    QList<QWidget*> m_tabOrder;
    QHash<QString, QString> m_comments;
    bool m_enabled;
};

class QDESIGNER_SHARED_EXPORT MetaDataBase: public QDesignerMetaDataBaseInterface
{
    Q_OBJECT
public:
    MetaDataBase(QDesignerFormEditorInterface *core, QObject *parent = 0);
    virtual ~MetaDataBase();

    virtual QDesignerFormEditorInterface *core() const;

    virtual QDesignerMetaDataBaseItemInterface *item(QObject *object) const;
    virtual void add(QObject *object);
    virtual void remove(QObject *object);

    virtual QList<QObject*> objects() const;

    void dump();

private slots:
    void slotDestroyed(QObject *object);

private:
    QDesignerFormEditorInterface *m_core;
    typedef QHash<QObject *, MetaDataBaseItem*> ItemMap;
    ItemMap m_items;
};

} // namespace qdesigner_internal

#endif // METADATABASE_H
