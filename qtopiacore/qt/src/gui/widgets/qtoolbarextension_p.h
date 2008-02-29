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

#ifndef QDYNAMICTOOLBAREXTENSION_P_H
#define QDYNAMICTOOLBAREXTENSION_P_H

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

#include "QtGui/qtoolbutton.h"

#ifndef QT_NO_TOOLBUTTON

class QToolBarExtension : public QToolButton
{
    Q_OBJECT
    Qt::Orientation orientation;

public:
    explicit QToolBarExtension(QWidget *parent);

    QSize sizeHint() const;

public Q_SLOTS:
    void setOrientation(Qt::Orientation o);
};

#endif // QT_NO_TOOLBUTTON

#endif // QDYNAMICTOOLBAREXTENSION_P_H
