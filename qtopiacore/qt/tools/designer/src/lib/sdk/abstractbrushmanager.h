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

#ifndef ABSTRACTBRUSHMANAGER_H
#define ABSTRACTBRUSHMANAGER_H

#include <QtDesigner/sdk_global.h>

#include <QtCore/qobject.h>
#include <QtCore/qmap.h>
#include <QtGui/qbrush.h>

QT_BEGIN_HEADER

class QDESIGNER_SDK_EXPORT QDesignerBrushManagerInterface : public QObject
{
    Q_OBJECT
public:
    QDesignerBrushManagerInterface(QObject *parentObject = 0) : QObject(parentObject) {}

    virtual QBrush brush(const QString &name) const = 0;
    virtual QMap<QString, QBrush> brushes() const = 0;
    virtual QString currentBrush() const = 0;

    virtual QString addBrush(const QString &name, const QBrush &brush) = 0;
    virtual void removeBrush(const QString &name) = 0;
    virtual void setCurrentBrush(const QString &name) = 0;

    virtual QPixmap brushPixmap(const QBrush &brush) const = 0;
signals:
    void brushAdded(const QString &name, const QBrush &brush);
    void brushRemoved(const QString &name);
    void currentBrushChanged(const QString &name, const QBrush &brush);

};

QT_END_HEADER

#endif
