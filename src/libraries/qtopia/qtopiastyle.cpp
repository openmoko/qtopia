/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#include <QPainter>

static int indicatorSize = 11;
static int exclusiveIndicatorSize = 10;

static int leftMargin = 6;
static int topMargin = 6;
static int rightMargin = 6;
static int bottomMargin = 6;
static int buttonMargin = 4;

static int horizontalSpacing = 6;
static int verticalSpacing = 6;

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

    // 8 pixels on a 100dpi screen
    indicatorSize = qRound(8.0 * dpi / 100.0);
    exclusiveIndicatorSize = qRound(8.5 * dpi / 100.0);

    // 3 pixels on a 100dpi screen
    leftMargin = qRound(3.0 * dpi / 100.0);
    topMargin = qRound(3.0 * dpi / 100.0);
    rightMargin = qRound(3.0 * dpi / 100.0);
    bottomMargin = qRound(3.0 * dpi / 100.0);
    buttonMargin = qRound(3.0 * dpi / 100.0);

    // 3 pixels on a 100dpi screen
    horizontalSpacing = qRound(3.0 * dpi / 100.0);
    verticalSpacing = qRound(3.0 * dpi / 100.0);
}

/*!
    Destroys the QtopiaStyle object.
*/
QtopiaStyle::~QtopiaStyle()
{
}

/*!
    \reimp
*/
int QtopiaStyle::pixelMetric(PixelMetric metric, const QStyleOption *option,
                            const QWidget *widget) const
{
    int ret;

    switch (metric) {
        case PM_LayoutLeftMargin:
            ret = leftMargin;
            break;

        case PM_LayoutTopMargin:
            ret = topMargin;
            break;

        case PM_LayoutRightMargin:
            ret = rightMargin;
            break;

        case PM_LayoutBottomMargin:
            ret = bottomMargin;
            break;

        case PM_LayoutHorizontalSpacing:
            ret = horizontalSpacing;
            break;

        case PM_LayoutVerticalSpacing:
            ret = verticalSpacing;
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
                    // We would like a 12x12 icon at 100dpi
                    size = (12 * QApplication::desktop()->screen()->logicalDpiY()+50) / 100;
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
        case QStyle::SH_Menu_Scrollable:
            ret = 1;
            break;
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

/*!
  Draws a rectangle \a r with rounded corners using \a painter.

  The \a xRnd and \a yRnd arguments specify how rounded the corners
  should be. 0 is angled corners, 99 is maximum roundedness.

  The roundness is scaled to maintain a similar appearance at different
  display resolutions.

  \sa QPainter::drawRoundRect()
*/
void QtopiaStyle::drawRoundRect(QPainter *painter, const QRectF &r, int xRnd, int yRnd)
{
    if (r.isEmpty())
        return;

    int dpi = QApplication::desktop()->screen()->logicalDpiY();
    painter->drawRoundRect(r, static_cast<int>(xRnd * dpi / (r.width()*2)), static_cast<int>(yRnd * dpi / (r.height()*2)));
}

/*!
  Draws a rectangle \a r with rounded corners using \a painter.

  The \a xRnd and \a yRnd arguments specify how rounded the corners
  should be. 0 is angled corners, 99 is maximum roundedness.

  The roundness is scaled to maintain a similar appearance at different
  display resolutions.

  \sa QPainter::drawRoundRect()
*/
void QtopiaStyle::drawRoundRect(QPainter *painter, const QRect &r, int xRnd, int yRnd)
{
    drawRoundRect(painter, QRectF(r), xRnd, yRnd);
}
