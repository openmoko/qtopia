/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtopiastyle.h"

#include <QApplication>
#include <QDesktopWidget>

static int childMargin = 4;
static int layoutSpacing = 4;
static int indicatorSize = 11;
static int exclusiveIndicatorSize = 10;
static int buttonMargin = 4;

/*!
    \class QtopiaStyle
    \mainclass
    \brief The QtopiaStyle class encapsulates the common Look and Feel of a Qtopia GUI.

    \ingroup appearance

    This class implements some of the widget's look and feel that is common to all 
    Qtopia GUI styles.

    \sa QWindowsXPStyle, QMacStyle, QPlastiqueStyle, QCDEStyle, QMotifStyle
*/

/*!
    Constructs a QtopiaStyle object.
*/
QtopiaStyle::QtopiaStyle()
{
    d = 0;
    int dpi = QApplication::desktop()->screen()->logicalDpiY();

    // 4 pixels on a 100dpi screen
    childMargin = qRound(4.0 * dpi / 100.0);
    layoutSpacing = qRound(4.0 * dpi / 100.0);
    buttonMargin = qRound(4.0 * dpi / 100.0);
    // 8 pixels on a 100dpi screen
    indicatorSize = qRound(8.0 * dpi / 100.0);
    // 8 pixels on a 100dpi screen
    exclusiveIndicatorSize = qRound(8.0 * dpi / 100.0);
}

/*!
    Destroys the QtopiaStyle object.
*/
QtopiaStyle::~QtopiaStyle()
{
}

/*!
    \obsolete
*/
void QtopiaStyle::setTheme(const QString& themeconfig)
{
    Q_UNUSED(themeconfig);
}

/*!
    \reimp
*/
int QtopiaStyle::pixelMetric(PixelMetric metric, const QStyleOption *option,
                            const QWidget *widget) const
{
    int ret;

    switch (metric) {
        case PM_DefaultTopLevelMargin:
            ret = 0;
            break;

        case PM_DefaultChildMargin:
            ret = childMargin;
            break;

        case PM_DefaultLayoutSpacing:
            ret = layoutSpacing;
            break;

        case PM_IndicatorWidth:
        case PM_IndicatorHeight:
            ret = indicatorSize;
            break;

        case PM_ExclusiveIndicatorWidth:
        case PM_ExclusiveIndicatorHeight:
            ret = exclusiveIndicatorSize;
            break;

        case PM_DefaultFrameWidth:
            ret = 1;
            break;

        case PM_ButtonMargin:
            ret = buttonMargin;
            break;

        case PM_ButtonIconSize:
        case PM_ToolBarIconSize:
        case PM_SmallIconSize: {
                static int size = 0;
                if (!size) {
                    // We would like a 14x14 icon at 100dpi
                    size = (14 * QApplication::desktop()->screen()->logicalDpiY()+50) / 100;
                }
                ret = size;
            }
            break;

        case PM_LargeIconSize:
        case PM_MessageBoxIconSize:
        case PM_IconViewIconSize: {
                static int size = 0;
                if (!size) {
                    // We would like a 28x28 icon at 100dpi
                    size = (28 * QApplication::desktop()->screen()->logicalDpiY()+50) / 100;
                }
                ret = size;
            }
            break;

        case PM_TabBarIconSize:
        case PM_ListViewIconSize: {
                static int size = 0;
                if (!size) {
                    // We would like a 18x18 icon at 100dpi
                    size = (18 * QApplication::desktop()->screen()->logicalDpiY()+50) / 100;
                }
                ret = size;
            }
            break;

        default:
            ret = QWindowsStyle::pixelMetric(metric, option, widget);
    }

    return ret;
}

/*!
    \reimp
*/
int QtopiaStyle::styleHint(StyleHint stylehint, const QStyleOption *option,
                      const QWidget *widget, QStyleHintReturn* returnData) const
{
    int ret = 0;
    switch (stylehint) {
        case QStyle::SH_ItemView_ShowDecorationSelected:
            ret = true;
            break;
        case QStyle::SH_ItemView_ActivateItemOnSingleClick:
            ret = true;
            break;
#if QT_VERSION >= 0x040200 // Doesn't work in earlier Qt
        case QStyle::SH_Menu_Scrollable:
            ret = 1;
            break;
#endif
        default:
            ret = QWindowsStyle::styleHint(stylehint, option, widget, returnData);
    }

    return ret;
}

/*!
    \reimp
*/
QPixmap QtopiaStyle::standardPixmap(StandardPixmap standardPixmap,
                const QStyleOption *option, const QWidget *widget) const
{
    switch (standardPixmap) {
        case QStyle::SP_MessageBoxInformation:
            return QPixmap(QLatin1String(":image/alert_info"));
        case QStyle::SP_MessageBoxWarning:
            return QPixmap(QLatin1String(":image/alert_warning"));
        default:
            return QWindowsStyle::standardPixmap(standardPixmap, option, widget);
    }
}

