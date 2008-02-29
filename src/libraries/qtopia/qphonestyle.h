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

#if defined(QTOPIA_PHONE) && !defined(PHONESTYLE_H)
#define PHONESTYLE_H

#include <qtopiastyle.h>

class QPhoneStylePrivate;

class QTOPIA_EXPORT QPhoneStyle : public QtopiaStyle
{
    Q_OBJECT
public:
    QPhoneStyle();
    virtual ~QPhoneStyle();

    void polish(QPalette &pal);
    void polish(QWidget *widget);
    void unpolish(QWidget *widget);
    void unpolish(QApplication *app);
    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;
    QSize sizeFromContents ( ContentsType type, const QStyleOption * option, const QSize & contentsSize, const QWidget * widget = 0 ) const;
    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                        const QWidget *w = 0) const;
    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const;
    void drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p, const QWidget *widget) const;
    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                        QPainter *p, const QWidget *widget) const;

    bool event(QEvent *e);

private:
    QPhoneStylePrivate *d;
};


#endif // PHONESTYLE_H
