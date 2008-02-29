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

#ifndef DEFAULT_ACTIONPROVIDER_H
#define DEFAULT_ACTIONPROVIDER_H

#include "formeditor_global.h"
#include "actionprovider_p.h"

#include <QtDesigner/QExtensionFactory>

#include <QtCore/QPair>
#include <QtCore/QRect>

class QLayoutWidget;
class QLayoutSupport;

namespace qdesigner_internal {

class FormWindow;

class QT_FORMEDITOR_EXPORT QDesignerActionProvider: public QObject, public QDesignerActionProviderExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerActionProviderExtension)
public:
    QDesignerActionProvider(QWidget *widget, QObject *parent = 0);
    virtual ~QDesignerActionProvider();

    virtual QRect actionGeometry(QAction *action) const;
    virtual QAction *actionAt(const QPoint &pos) const;

    virtual void adjustIndicator(const QPoint &pos);

    Qt::Orientation orientation() const;

private:
    QWidget *m_widget;
    QWidget *m_indicator;
};

class QT_FORMEDITOR_EXPORT QDesignerActionProviderFactory: public QExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(QAbstractExtensionFactory)
public:
    QDesignerActionProviderFactory(QExtensionManager *parent = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

} // namespace qdesigner_internal

#endif // DEFAULT_ACTIONPROVIDER_H
