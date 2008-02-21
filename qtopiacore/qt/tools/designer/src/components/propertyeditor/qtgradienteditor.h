/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.1, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
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
