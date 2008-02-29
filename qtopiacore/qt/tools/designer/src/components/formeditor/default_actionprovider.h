/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
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
