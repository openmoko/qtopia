/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
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

#ifndef LAYOUTINFO_H
#define LAYOUTINFO_H

#include "shared_global_p.h"

#include <QtCore/QList>

class QWidget;
class QLayout;
class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT LayoutInfo
{
public:
    enum Type
    {
        HBox,
        VBox,
        Grid,
        Stacked,
        NoLayout
    };

    static void deleteLayout(QDesignerFormEditorInterface *core, QWidget *widget);
    static Type layoutType(QDesignerFormEditorInterface *core, QWidget *w, QLayout *&layout);
    static Type layoutType(QDesignerFormEditorInterface *core, QLayout *layout);
    static Type layoutType(QDesignerFormEditorInterface *core, QWidget *w);
    static QWidget *layoutParent(QDesignerFormEditorInterface *core, QLayout *layout);
    static bool isWidgetLaidout(QDesignerFormEditorInterface *core, QWidget *widget);

    static QLayout *managedLayout(QDesignerFormEditorInterface *core, QWidget *widget);

    class Interval
    {
    public:
        int v1, v2;
        inline Interval(int _v1 = 0, int _v2 = 0)
            : v1(_v1), v2(_v2) {}
        bool operator < (const Interval &other) const
            { return v1 < other.v1; }
    };
    typedef QList<Interval> IntervalList;
    static void cells(QLayout *layout, IntervalList *rows, IntervalList *columns);
};

} // namespace qdesigner_internal

#endif // LAYOUTINFO_H
