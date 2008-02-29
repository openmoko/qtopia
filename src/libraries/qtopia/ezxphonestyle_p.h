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

#ifdef EZX_A780
#ifndef _HAVE_EZXPHONESTYLE_H_
#define _HAVE_EZXPHONESTYLE_H_

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include <QPhoneStyle>
class EzXPhoneStyle : public QPhoneStyle
{
public:
    EzXPhoneStyle();
    void drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p,
                                const QWidget *widget) const;
    int pixelMetric(PixelMetric metric, const QStyleOption *option,
                            const QWidget *widget) const;
    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt,
                                SubControl sc, const QWidget *w) const;

    virtual int scrollLineHeight() const;
private:
    void loadPixmaps() const;
};

#endif
#endif
