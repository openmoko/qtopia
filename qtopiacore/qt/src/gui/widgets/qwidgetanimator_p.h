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

#ifndef QWIDGET_ANIMATOR_P_H
#define QWIDGET_ANIMATOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include <QRect>
#include <QMap>

class QWidget;
class QTimer;
class QTime;

class QWidgetAnimator : public QObject
{
    Q_OBJECT
public:
    QWidgetAnimator(QObject *parent = 0);
    ~QWidgetAnimator();
    void animate(QWidget *widget, const QRect &final_geometry, bool animate);
    bool animating() const;
    bool animating(QWidget *widget);

signals:
    void finished(QWidget *widget);
    void finishedAll();

private slots:
    void animationStep();

private:
    struct AnimationItem {
        AnimationItem(QWidget *_widget = 0, const QRect &_r1 = QRect(),
                        const QRect &_r2 = QRect())
            : widget(_widget), r1(_r1), r2(_r2), step(0) {}
        QWidget *widget;
        QRect r1, r2;
        int step;
    };
    typedef QMap<QWidget*, AnimationItem> AnimationMap;
    AnimationMap m_animation_map;
    QTimer *m_timer;
    QTime *m_time;
};

#endif // QWIDGET_ANIMATOR_P_H
