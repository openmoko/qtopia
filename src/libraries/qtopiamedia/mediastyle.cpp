/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "mediastyle.h"

void MediaStyle::drawControl( ControlElement ce, const QStyleOption *opt, QPainter *p, const QWidget *widget ) const
{
    switch( ce )
    {
    // Slimline progress bar
    case CE_ProgressBarGroove:
        {
        QLinearGradient gradient( 0, 0, 0, opt->rect.height() );
        QColor color = opt->palette.color( QPalette::Shadow );
        gradient.setColorAt( 0.3, color );
        gradient.setColorAt( 0.9, color.light( 300 ) );

        p->setPen( color.light( 300 ) );
        p->setBrush( gradient );
        p->drawRect( opt->rect.adjusted( 0, 0, -1, -1 ) ); // ### isValid
        }
        break;
    case CE_ProgressBarContents:
        {
        const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar*>(opt);
        if( pb ) {
            QRect rect = pb->rect;
            QRect progress;
            if( pb->maximum > pb->minimum ) {
                progress = rect.adjusted( 0, 0,
                    -rect.width() + rect.width()*(pb->progress - pb->minimum)/(pb->maximum - pb->minimum), 0 );
                progress.adjust( 1, 1, -1, -1 );
            }

            if( progress.isValid() ) {
                QLinearGradient gradient( 0, 0, 0, pb->rect.height() );
                QColor color = opt->palette.color( QPalette::Highlight );
                gradient.setColorAt( 0.3, color );
                gradient.setColorAt( 0.9, color.dark( 150 ) );

                p->setPen( Qt::NoPen );
                p->setBrush( gradient );
                p->drawRect( progress );
            }
        }
        }
        break;
    default:
        QWindowsStyle::drawControl( ce, opt, p, widget );
        break;
    }
}

int MediaStyle::pixelMetric( PixelMetric pm, const QStyleOption *opt, const QWidget *widget ) const
{
    if( pm == PM_ProgressBarChunkWidth ) {
        return 1;
    }

    return QWindowsStyle::pixelMetric( pm, opt, widget );
}
