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

#ifndef __QTOPIA_HELIXDIRECTPAINTERWIDGET_H
#define __QTOPIA_HELIXDIRECTPAINTERWIDGET_H


#include <QDirectPainter>

#include "helixvideosurface.h"



class DirectPainterVideoWidget :
    public QDirectPainter,
    public PaintObserver
{
public:
    DirectPainterVideoWidget(GenericVideoSurface* surface,
                             QWidget* parent = 0);
    ~DirectPainterVideoWidget();

    // Observer
    void paintNotification();

    static int isSupported();

private:
    void regionChanged(const QRegion &exposedRegion);
    void calc(QRegion const& region);
    void paint();

    GenericVideoSurface *m_surface;

    bool        m_isVisible;
    bool        m_firstPaintCalc;
    bool        m_painting;
    int         m_black;
    QRect       m_destRect;
    QRegion     m_savedRegion;
    QRegion     m_blackRegion;
#ifndef QTOPIA_NO_MEDIAVIDEOSCALING
    QImage*     m_frameBufferImage;
#else
    QPoint      m_destTopLeft;
#endif
};

#endif  // __QTOPIA_HELIXDIRECTPAINTERWIDGET_H

