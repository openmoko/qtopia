/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
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

#ifndef QDESIGNER_GRID_H
#define QDESIGNER_GRID_H

#include "shared_global_p.h"

#include <QtCore/QVariantMap>

class QWidget;
class QPaintEvent;

namespace qdesigner_internal {

// Designer grid which is able to serialize to QVariantMap
class QDESIGNER_SHARED_EXPORT Grid
{
public:
    Grid();

    bool fromVariantMap(const QVariantMap& vm);

    void addToVariantMap(QVariantMap& vm, bool forceKeys = false) const;
    QVariantMap toVariantMap(bool forceKeys = false) const;

    inline bool visible() const   { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

    inline bool snapX() const     { return m_snapX; }
    void setSnapX(bool snap)      { m_snapX = snap; }

    inline bool snapY() const     { return m_snapY; }
    void setSnapY(bool snap)      { m_snapY = snap; }

    inline int deltaX() const     { return m_deltaX; }
    void setDeltaX(int dx)        { m_deltaX = dx; }

    inline int deltaY() const     { return m_deltaY; }
    void setDeltaY(int dy)        { m_deltaY = dy; }

    void paint(QWidget *widget, QPaintEvent *e, bool needFrame = false) const;

    QPoint snapPoint(const QPoint &p) const;

    int widgetHandleAdjustX(int x) const;
    int widgetHandleAdjustY(int y) const;

private:
    int snapValue(int value, int grid) const;
    bool m_visible;
    bool m_snapX;
    bool m_snapY;
    int m_deltaX;
    int m_deltaY;
};
} // namespace qdesigner_internal

#endif // QDESIGNER_GRID_H
