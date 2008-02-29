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

#ifndef QDYNAMICTOOLBARHANDLE_P_H
#define QDYNAMICTOOLBARHANDLE_P_H

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

#include "QtGui/qwidget.h"

#ifndef QT_NO_TOOLBAR

class QToolBar;

class QToolBarHandle : public QWidget
{
    Q_OBJECT
    Qt::Orientation orient;
    struct DragState {
	QPoint offset;
	bool canDrop;
    };
    DragState *state;

public:
    explicit QToolBarHandle(QToolBar *parent);

    Qt::Orientation orientation() const;

    QSize sizeHint() const;

    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *e);

public Q_SLOTS:
    void setOrientation(Qt::Orientation orientation);
};

#endif // QT_NO_TOOLBAR

#endif // QDYNAMICTOOLBARHANDLE_P_H
