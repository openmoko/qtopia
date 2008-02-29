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

#ifndef QDECORATIONDEFAULT_QWS_H
#define QDECORATIONDEFAULT_QWS_H

#include <QtGui/qdecoration_qws.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#if !defined(QT_NO_QWS_DECORATION_DEFAULT) || defined(QT_PLUGIN)

#define CORNER_GRAB 16
#define BORDER_WIDTH  4
#define BOTTOM_BORDER_WIDTH BORDER_WIDTH

class Q_GUI_EXPORT QDecorationDefault : public QDecoration
{
public:
    QDecorationDefault();
    virtual ~QDecorationDefault();

    virtual QRegion region(const QWidget *widget, const QRect &rect, int decorationRegion = All);
    virtual bool paint(QPainter *painter, const QWidget *widget, int decorationRegion = All,
                       DecorationState state = Normal);

protected:
    virtual int titleBarHeight(const QWidget *widget);

    virtual void paintButton(QPainter *painter, const QWidget *widget, int buttonRegion,
                             DecorationState state, const QPalette &pal);
    virtual QPixmap pixmapFor(const QWidget *widget, int decorationRegion, int &xoff, int &yoff);
    virtual const char **xpmForRegion(int region);

    int menu_width;
    int help_width;
    int close_width;
    int minimize_width;
    int maximize_width;
    int normalize_width;

private:
    static QPixmap *staticHelpPixmap;
    static QPixmap *staticMenuPixmap;
    static QPixmap *staticClosePixmap;
    static QPixmap *staticMinimizePixmap;
    static QPixmap *staticMaximizePixmap;
    static QPixmap *staticNormalizePixmap;

};

#endif // QT_NO_QWS_DECORATION_DEFAULT
QT_END_HEADER

#endif // QDECORATIONDEFAULT_QWS_H
