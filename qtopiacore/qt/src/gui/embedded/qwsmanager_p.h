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

#ifndef QWSMANAGER_P_H
#define QWSMANAGER_P_H

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

#include "QtGui/qregion.h"
#include "QtGui/qdecoration_qws.h"

#ifndef QT_NO_QWS_MANAGER

#include "QtCore/qhash.h"

class QWidget;
class QMenu;

class QWSManagerPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWSManager)
public:
    QWSManagerPrivate();

    int activeRegion;
    QWidget *managed;
    QMenu *popup;

    enum MenuAction {
        NormalizeAction,
        TitleAction,
        BottomRightAction,
        MinimizeAction,
        MaximizeAction,
        CloseAction,
        LastMenuAction
    };
    QAction *menuActions[LastMenuAction];

    static QWidget *active;
    static QPoint mousePos;

    // Region caching to avoid getting a regiontype's
    // QRegion for each mouse move event
    int previousRegionType;
    bool previousRegionRepainted; // Hover/Press handled
    struct RegionCaching {
        int regionType;
        QRegion region;
        Qt::WindowFlags windowFlags;
        QRect windowGeometry;
    } cached_region;

    bool newCachedRegion(const QPoint &pos);
    int cachedRegionAt()
    { return cached_region.regionType; }

    void dirtyRegion(int decorationRegion,
                     QDecoration::DecorationState state,
                     const QRegion &clip = QRegion());
    void paint(QPaintDevice *paintDevice, const QRegion &region);

    QList<int> dirtyRegions;
    QList<QDecoration::DecorationState> dirtyStates;
};

#endif // QT_NO_QWS_MANAGER

#endif // QWSMANAGER_P_H
