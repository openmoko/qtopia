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

#ifndef ABSTRACTMETADATABASE_H
#define ABSTRACTMETADATABASE_H

#include <QtDesigner/sdk_global.h>

#include <QtCore/QObject>
#include <QtCore/QList>

QT_BEGIN_HEADER

class QCursor;
class QWidget;

class QDesignerFormEditorInterface;

class QDesignerMetaDataBaseItemInterface
{
public:
    virtual ~QDesignerMetaDataBaseItemInterface() {}

    virtual QString name() const = 0;
    virtual void setName(const QString &name) = 0;

    virtual QList<QWidget*> tabOrder() const = 0;
    virtual void setTabOrder(const QList<QWidget*> &tabOrder) = 0;

    virtual bool enabled() const = 0;
    virtual void setEnabled(bool b) = 0;
};

class QDESIGNER_SDK_EXPORT QDesignerMetaDataBaseInterface: public QObject
{
    Q_OBJECT
public:
    QDesignerMetaDataBaseInterface(QObject *parent = 0);
    virtual ~QDesignerMetaDataBaseInterface();

    virtual QDesignerMetaDataBaseItemInterface *item(QObject *object) const = 0;
    virtual void add(QObject *object) = 0;
    virtual void remove(QObject *object) = 0;

    virtual QList<QObject*> objects() const = 0;

    virtual QDesignerFormEditorInterface *core() const = 0;

Q_SIGNALS:
    void changed();
};

QT_END_HEADER

#endif // ABSTRACTMETADATABASE_H
