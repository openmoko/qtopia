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

#ifndef QDYNAMICTOOLBARSEPARATOR_P_H
#define QDYNAMICTOOLBARSEPARATOR_P_H

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

class QToolBarSeparator : public QWidget
{
    Q_OBJECT
    Qt::Orientation orient;

public:
    explicit QToolBarSeparator(QToolBar *parent);

    Qt::Orientation orientation() const;

    QSize sizeHint() const;

    void paintEvent(QPaintEvent *);

public Q_SLOTS:
    void setOrientation(Qt::Orientation orientation);
};

#endif // QT_NO_TOOLBAR

#endif // QDYNAMICTOOLBARSEPARATOR_P_H
