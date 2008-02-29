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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef INVISIBLE_WIDGET_H
#define INVISIBLE_WIDGET_H

#include "shared_global_p.h"

#include <QtGui/QWidget>

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT InvisibleWidget: public QWidget
{
    Q_OBJECT
public:
    InvisibleWidget(QWidget *parent = 0);
};

} // namespace qdesigner_internal

#endif // INVISIBLE_WIDGET_H
