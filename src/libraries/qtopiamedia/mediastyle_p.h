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

#ifndef MEDIASTYLE_H
#define MEDIASTYLE_H

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

#include <qtopiabase/qtopiaglobal.h>

#include <QtGui>

class QTOPIAMEDIA_EXPORT MediaStyle : public QCommonStyle
{
public:
    void drawControl( ControlElement ce, const QStyleOption *opt, QPainter *p, const QWidget *widget ) const;

    int pixelMetric( PixelMetric pm, const QStyleOption *opt, const QWidget *widget ) const;

private:
    mutable QPixmap m_groovebuffer;
    mutable QPixmap m_barbuffer;
    mutable QPixmap m_silhouettebuffer;
};

#endif // MEDIASTYLE_H
