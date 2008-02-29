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
    static int adjustPoint(int x, int dx);

private:
    QWidget *widget;
    Type type;
    QPoint origPressPos;
    FormWindow *formWindow;
    WidgetSelection *sel;
    QRect geom, origGeom;
    bool active;
};

class QT_FORMEDITOR_EXPORT WidgetSelection: public QObject
{
    Q_OBJECT
public:
    WidgetSelection(FormWindow *parent, QHash<QWidget *, WidgetSelection *> *selDict);

    void setWidget(QWidget *w, bool updateDict = true);
    bool isUsed() const;

    void updateGeometry();
    void hide();
    void show();
    void update();

    QWidget *widget() const;

    QDesignerTaskMenuExtension *taskMenuExtension() const
    { return taskMenu; }

    QDesignerFormEditorInterface *core() const;

    virtual bool eventFilter(QObject *object, QEvent *event);

protected:
    QHash<int, WidgetHandle*> handles;
    InvisibleWidget *m_topWidget;
    QPointer<QWidget> wid;
    FormWindow *formWindow;
    QHash<QWidget *, WidgetSelection *> *selectionDict;
    QDesignerTaskMenuExtension *taskMenu;
};

}  // namespace qdesigner_internal


#endif // WIDGETSELECTION_H
