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

#ifndef QTABBAR_P_H
#define QTABBAR_P_H

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

#include "qtabbar.h"
#include "qicon.h"
#include "qtoolbutton.h"
#include "private/qwidget_p.h"

#ifndef QT_NO_TABBAR

class QTabBarPrivate  : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QTabBar)
public:
    QTabBarPrivate()
        :currentIndex(-1), pressedIndex(-1),
         shape(QTabBar::RoundedNorth),
         layoutDirty(false), drawBase(true), scrollOffset(0){}

    int currentIndex;
    int pressedIndex;
    QTabBar::Shape shape;
    bool layoutDirty;
    bool drawBase;
    int scrollOffset;

    struct Tab {
        inline Tab():enabled(true), shortcutId(0){}
        inline Tab(const QIcon &ico, const QString &txt):enabled(true), shortcutId(0), text(txt), icon(ico){}
        bool enabled;
        int shortcutId;
        QString text;
#ifndef QT_NO_TOOLTIP
        QString toolTip;
#endif
#ifndef QT_NO_WHATSTHIS
        QString whatsThis;
#endif
        QIcon icon;
        QRect rect;
        QRect minRect;
        QRect maxRect;

        QColor textColor;
        QVariant data;
    };
    QList<Tab> tabList;

    void init();
    int extraWidth() const;

    Tab *at(int index);
    const Tab *at(int index) const;

    int indexAtPos(const QPoint &p) const;

    inline bool validIndex(int index) const { return index >= 0 && index < tabList.count(); }

    QSize minimumTabSizeHint(int index);

    QToolButton* rightB; // right or bottom
    QToolButton* leftB; // left or top
    void _q_scrollTabs(); // private slot
    QRect hoverRect;

    void refresh();
    void layoutTabs();

    void makeVisible(int index);
    QStyleOptionTabV2 getStyleOption(int tab) const;
    QSize iconSize;
    Qt::TextElideMode elideMode;
    bool useScrollButtons;
};

#endif

#endif
