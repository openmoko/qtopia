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

#ifndef QDECORATIONSTYLED_QWS_H
#define QDECORATIONSTYLED_QWS_H

#include <QtGui/qdecorationdefault_qws.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#if !defined(QT_NO_QWS_DECORATION_STYLED) || defined(QT_PLUGIN)

class Q_GUI_EXPORT QDecorationStyled : public QDecorationDefault
{
public:
    QDecorationStyled();
    virtual ~QDecorationStyled();

    QRegion region(const QWidget *widget, const QRect &rect, int decorationRegion = All);
    bool paint(QPainter *painter, const QWidget *widget, int decorationRegion = All,
               DecorationState state = Normal);
    int titleBarHeight(const QWidget *widget);
};

#endif // QT_NO_QWS_DECORATION_STYLED

QT_END_HEADER

#endif // QDECORATIONSTYLED_QWS_H
