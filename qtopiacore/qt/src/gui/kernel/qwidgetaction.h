/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QWIDGETACTION_H
#define QWIDGETACTION_H

#include <QtGui/qaction.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_ACTION

class QWidgetActionPrivate;

class Q_GUI_EXPORT QWidgetAction : public QAction
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWidgetAction)

public:
    explicit QWidgetAction(QObject *parent);
    virtual ~QWidgetAction();
    
    void setDefaultWidget(QWidget *w);
    QWidget *defaultWidget() const;

    QWidget *requestWidget(QWidget *parent);
    void releaseWidget(QWidget *widget);
    
protected:
    virtual bool event(QEvent *);
    virtual QWidget *createWidget(QWidget *parent);
    virtual void deleteWidget(QWidget *widget);
    QList<QWidget *> createdWidgets() const;

private:
    Q_DISABLE_COPY(QWidgetAction)
    Q_PRIVATE_SLOT(d_func(), void _q_widgetDestroyed(QObject *))
    friend class QToolBar;
};

#endif // QT_NO_ACTION

QT_END_HEADER

#endif // QWIDGETACTION_H
