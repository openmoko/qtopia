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

#ifndef QSLIDER_H
#define QSLIDER_H

#include <QtGui/qabstractslider.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_SLIDER

class QSliderPrivate;
class QStyleOptionSlider;
class Q_GUI_EXPORT QSlider : public QAbstractSlider
{
    Q_OBJECT

    Q_ENUMS(TickPosition)
    Q_PROPERTY(TickPosition tickPosition READ tickPosition WRITE setTickPosition)
    Q_PROPERTY(int tickInterval READ tickInterval WRITE setTickInterval)

public:
    enum TickPosition {
        NoTicks = 0,
        TicksAbove = 1,
        TicksLeft = TicksAbove,
        TicksBelow = 2,
        TicksRight = TicksBelow,
        TicksBothSides = 3

#if defined(QT3_SUPPORT) && !defined(Q_MOC_RUN)
        ,NoMarks = NoTicks,
        Above = TicksAbove,
        Left = TicksAbove,
        Below = TicksBelow,
        Right = TicksRight,
        Both = TicksBothSides
#endif
    };

    explicit QSlider(QWidget *parent = 0);
    explicit QSlider(Qt::Orientation orientation, QWidget *parent = 0);

    ~QSlider();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void setTickPosition(TickPosition position);
    TickPosition tickPosition() const;

    void setTickInterval(int ti);
    int tickInterval() const;

    bool event(QEvent *event);

protected:
    void paintEvent(QPaintEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QSlider(QWidget *parent, const char *name);
    QT3_SUPPORT_CONSTRUCTOR QSlider(Qt::Orientation, QWidget *parent, const char *name);
    QT3_SUPPORT_CONSTRUCTOR QSlider(int minValue, int maxValue, int pageStep, int value,
                                  Qt::Orientation orientation,
                                  QWidget *parent = 0, const char *name = 0);
    inline QT3_SUPPORT void setTickmarks(TickPosition position) { setTickPosition(position); }
    inline QT3_SUPPORT TickPosition tickmarks() const { return tickPosition(); }
public Q_SLOTS:
    inline QT_MOC_COMPAT void addStep() { triggerAction(SliderSingleStepAdd); };
    inline QT_MOC_COMPAT void subtractStep() { triggerAction(SliderSingleStepSub); };
#endif

private:
    friend Q_GUI_EXPORT QStyleOptionSlider qt_qsliderStyleOption(QSlider *slider);
    
    Q_DISABLE_COPY(QSlider)
    Q_DECLARE_PRIVATE(QSlider)
};

#endif // QT_NO_SLIDER

QT_END_HEADER

#endif // QSLIDER_H
