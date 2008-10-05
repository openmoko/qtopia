/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef EZXPHONESTYLE_P_H
#define EZXPHONESTYLE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifdef EZX_A780

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
