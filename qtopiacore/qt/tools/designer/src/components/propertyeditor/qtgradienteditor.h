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

#ifndef QTGRADIENTEDITOR_H
#define QTGRADIENTEDITOR_H

#include <QWidget>

namespace qdesigner_internal {

class QtGradientEditor : public QWidget
{
    Q_OBJECT
public:
    QtGradientEditor(QWidget *parent = 0);
    ~QtGradientEditor();

    void setGradient(const QGradient &gradient);
    QGradient gradient() const;

signals:

    void gradientChanged(const QGradient &gradient);

private:
    class QtGradientEditorPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtGradientEditor)
    Q_DISABLE_COPY(QtGradientEditor)
    Q_PRIVATE_SLOT(d_func(), void slotGradientStopsChanged(const QGradientStops &stops))
    Q_PRIVATE_SLOT(d_func(), void slotTypeChanged(int type))
    Q_PRIVATE_SLOT(d_func(), void slotSpreadChanged(int type))
    Q_PRIVATE_SLOT(d_func(), void slotStartLinearXChanged())
    Q_PRIVATE_SLOT(d_func(), void slotStartLinearYChanged())
    Q_PRIVATE_SLOT(d_func(), void slotEndLinearXChanged())
    Q_PRIVATE_SLOT(d_func(), void slotEndLinearYChanged())
    Q_PRIVATE_SLOT(d_func(), void slotCentralRadialXChanged())
    Q_PRIVATE_SLOT(d_func(), void slotCentralRadialYChanged())
    Q_PRIVATE_SLOT(d_func(), void slotFocalRadialXChanged())
    Q_PRIVATE_SLOT(d_func(), void slotFocalRadialYChanged())
    Q_PRIVATE_SLOT(d_func(), void slotRadiusRadialChanged())
    Q_PRIVATE_SLOT(d_func(), void slotCentralConicalXChanged())
    Q_PRIVATE_SLOT(d_func(), void slotCentralConicalYChanged())
    Q_PRIVATE_SLOT(d_func(), void slotAngleConicalChanged())
    Q_PRIVATE_SLOT(d_func(), void startLinearChanged(const QPointF &))
    Q_PRIVATE_SLOT(d_func(), void endLinearChanged(const QPointF &))
    Q_PRIVATE_SLOT(d_func(), void centralRadialChanged(const QPointF &))
    Q_PRIVATE_SLOT(d_func(), void focalRadialChanged(const QPointF &))
    Q_PRIVATE_SLOT(d_func(), void radiusRadialChanged(qreal))
    Q_PRIVATE_SLOT(d_func(), void centralConicalChanged(const QPointF &))
    Q_PRIVATE_SLOT(d_func(), void angleConicalChanged(qreal))
};

}

#endif
