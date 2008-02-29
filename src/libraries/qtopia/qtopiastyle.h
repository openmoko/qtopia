/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef QTOPIASTYLE_H
#define QTOPIASTYLE_H

#include <qwindowsstyle.h>
#include <qtopiaglobal.h>

class QtopiaStylePrivate;

class QTOPIA_EXPORT QtopiaStyle : public QWindowsStyle
{
    Q_OBJECT
public:
    QtopiaStyle();
    virtual ~QtopiaStyle();

    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;

    virtual int styleHint(StyleHint stylehint, const QStyleOption *opt = 0,
                      const QWidget *widget = 0, QStyleHintReturn* returnData = 0) const;
    virtual QPixmap standardPixmap(StandardPixmap standardPixmap,
                    const QStyleOption *option=0, const QWidget *widget=0) const;

    static void drawRoundRect(QPainter *p, const QRect &r, int xRnd, int yRnd);
    static void drawRoundRect(QPainter *p, const QRectF &r, int xRnd, int yRnd);

protected:
    QtopiaStylePrivate *d;
};

#endif // QTOPIASTYLE_H
