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
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef WIDGETSELECTION_H
#define WIDGETSELECTION_H

#include "formeditor_global.h"
#include <invisible_widget_p.h>

#include <QtCore/QHash>
#include <QtCore/QPointer>

class QDesignerTaskMenuExtension;
class QDesignerFormEditorInterface;
class QMouseEvent;
class QPaintEvent;

namespace qdesigner_internal {

class FormWindow;
class WidgetSelection;

class QT_FORMEDITOR_EXPORT WidgetHandle: public InvisibleWidget
{
    Q_OBJECT
public:
    enum Type
    {
        LeftTop,
        Top,
        RightTop,
        Right,
        RightBottom,
        Bottom,
        LeftBottom,
        Left,
        TaskMenu,

        TypeCount
    };

    WidgetHandle(FormWindow *parent, Type t, WidgetSelection *s);
    void setWidget(QWidget *w);
    void setActive(bool a);
    void updateCursor();

    void setEnabled(bool) {}

    QDesignerFormEditorInterface *core() const;

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

private:
    void trySetGeometry(QWidget *w, int x, int y, int width, int height);
    void tryResize(QWidget *w, int width, int height);

private:
    QWidget *m_widget;
    const Type m_type;
    QPoint m_origPressPos;
    FormWindow *m_formWindow;
    WidgetSelection *m_sel;
    QRect m_geom, m_origGeom;
    bool m_active;
};

class QT_FORMEDITOR_EXPORT WidgetSelection: public QObject
{
    Q_OBJECT
public:
    WidgetSelection(FormWindow *parent);

    void setWidget(QWidget *w);
    bool isUsed() const;

    void updateGeometry();
    void hide();
    void show();
    void update();

    QWidget *widget() const;

    QDesignerTaskMenuExtension *taskMenuExtension() const
    { return m_taskMenu; }

    QDesignerFormEditorInterface *core() const;

    virtual bool eventFilter(QObject *object, QEvent *event);

protected:
    QHash<int, WidgetHandle*> m_handles;
    InvisibleWidget *m_topWidget;
    QPointer<QWidget> m_wid;
    FormWindow *m_formWindow;
    QDesignerTaskMenuExtension *m_taskMenu;
};

}  // namespace qdesigner_internal


#endif // WIDGETSELECTION_H
