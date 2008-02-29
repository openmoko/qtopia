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

#ifndef DEFAULT_LAYOUTDECORATION_H
#define DEFAULT_LAYOUTDECORATION_H

#include "formeditor_global.h"
#include <QtDesigner/layoutdecoration.h>

#include <QtDesigner/default_extensionfactory.h>

#include <QtCore/QPair>
#include <QtCore/QRect>

class QLayoutWidget;
class QLayoutSupport;

namespace qdesigner_internal {

class FormWindow;

class QT_FORMEDITOR_EXPORT QDesignerLayoutDecoration: public QObject, public QDesignerLayoutDecorationExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerLayoutDecorationExtension)
public:
    QDesignerLayoutDecoration(QLayoutWidget *widget, QObject *parent = 0);
    QDesignerLayoutDecoration(FormWindow *formWindow, QWidget *widget, QObject *parent = 0);
    virtual ~QDesignerLayoutDecoration();

    virtual QList<QWidget*> widgets(QLayout *layout) const;

    virtual QRect itemInfo(int index) const;
    virtual int indexOf(QWidget *widget) const;
    virtual int indexOf(QLayoutItem *item) const;

    virtual InsertMode currentInsertMode() const;
    virtual int currentIndex() const;
    virtual QPair<int, int> currentCell() const;
    virtual void insertWidget(QWidget *widget, const QPair<int, int> &cell);
    virtual void removeWidget(QWidget *widget);

    virtual void insertRow(int row);
    virtual void insertColumn(int column);
    virtual void simplify();

    virtual int findItemAt(const QPoint &pos) const;
    virtual int findItemAt(int row, int column) const;
    virtual void adjustIndicator(const QPoint &pos, int index);

private:
    QLayoutSupport *m_layoutSupport;
};

class QT_FORMEDITOR_EXPORT QDesignerLayoutDecorationFactory: public QExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(QAbstractExtensionFactory)
public:
    QDesignerLayoutDecorationFactory(QExtensionManager *parent = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

}  // namespace qdesigner_internal


#endif // DEFAULT_LAYOUTDECORATION_H
