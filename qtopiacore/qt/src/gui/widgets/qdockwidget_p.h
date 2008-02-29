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

#ifndef QDYNAMICDOCKWIDGET_P_H
#define QDYNAMICDOCKWIDGET_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtGui/qstyleoption.h"
#include "private/qwidget_p.h"
#include "QtGui/qboxlayout.h"

#ifndef QT_NO_DOCKWIDGET

class QGridLayout;
class QWidgetResizeHandler;
class QRubberBand;
class QDockWidgetTitleButton;
class QSpacerItem;
class QDockWidgetItem;

//We need access to insertItem and addChildWidget in QDockWidget
class QDockWidgetBoxLayout : public QVBoxLayout
{
public:
    QDockWidgetBoxLayout(QWidget *parent = 0)
        : QVBoxLayout(parent) {}
#ifdef Q_NO_USING_KEYWORD
    inline void addChildWidget(QWidget *widget) { QVBoxLayout::addChildWidget(widget); }
    inline void insertItem(int index, QLayoutItem *item) { QVBoxLayout::insertItem(index, item); }
#else
    using QVBoxLayout::addChildWidget;
    using QVBoxLayout::insertItem;
#endif
};

class QDockWidgetPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QDockWidget)

    struct DragState {
        QPoint pressPos;
        bool dragging;
        QWidgetItem *widgetItem;
        QList<int> pathToGap;
        bool ownWidgetItem;
    };

public:
    inline QDockWidgetPrivate()
	: QWidgetPrivate(), state(0), widget(0),
          features(QDockWidget::DockWidgetClosable
                   | QDockWidget::DockWidgetMovable
                   | QDockWidget::DockWidgetFloatable),
          allowedAreas(Qt::AllDockWidgetAreas), top(0), box(0),
          topSpacer(0), floatButton(0), closeButton(0), resizer(0)
    { }

    void init();
    void _q_toggleView(bool); // private slot
    void _q_toggleTopLevel(); // private slot

    QStyleOptionDockWidget getStyleOption();

    void updateButtons();
    void relayout();
    DragState *state;

    QWidget *widget;

    QDockWidget::DockWidgetFeatures features;
    Qt::DockWidgetAreas allowedAreas;

    QGridLayout *top;
    QDockWidgetBoxLayout *box;
    QSpacerItem *topSpacer;
    QRect titleArea;
    QDockWidgetTitleButton *floatButton;
    QDockWidgetTitleButton *closeButton;

    QWidgetResizeHandler *resizer;
#ifndef QT_NO_ACTION
    QAction *toggleViewAction;
#endif

    QMainWindow *findMainWindow(QWidget *widget) const;

    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void unplug(const QRect &rect);
    void plug(const QRect &rect);
};

#endif // QT_NO_DOCKWIDGET

#endif // QDYNAMICDOCKWIDGET_P_H
