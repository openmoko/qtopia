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

#include "previewwidget.h"
#include <QtEvents>

PreviewWidget::PreviewWidget( QWidget *parent, const char *name )
    : PreviewWidgetBase( parent, name )
{
    // install event filter on child widgets
    QObjectList l = queryList("QWidget");
    for (int i = 0; i < l.size(); ++i) {
        QObject * obj = l.at(i);
        obj->installEventFilter(this);
        ((QWidget*)obj)->setFocusPolicy(Qt::NoFocus);
    }
}


void PreviewWidget::closeEvent(QCloseEvent *e)
{
    e->ignore();
}


bool PreviewWidget::eventFilter(QObject *, QEvent *e)
{
    switch ( e->type() ) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::Enter:
    case QEvent::Leave:
        return true; // ignore;
    default:
        break;
    }
    return false;
}
