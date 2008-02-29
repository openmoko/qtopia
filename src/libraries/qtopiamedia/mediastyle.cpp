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

#include "mediastyle.h"

static QPixmap generate_progress_bar( const QColor& start, const QColor& end, const QSize& size )
{
    QLinearGradient gradient( 0, 0, 0, size.height() );
    gradient.setColorAt( 0.3, start );
    gradient.setColorAt( 0.9, end );

    QPixmap buffer = QPixmap( size );
    buffer.fill( QColor( 0, 0, 0, 0 ) );

    QPainter painter( &buffer );
    painter.setPen( Qt::NoPen );
    painter.setBrush( gradient );
    painter.drawRect( buffer.rect() );

    return buffer;
}

void MediaStyle::drawControl( ControlElement ce, const QStyleOption *opt, QPainter *p, const QWidget *widget ) const
{
    switch( ce )
    {
    // Slimline progress bar
    case CE_ProgressBarGroove:
        {
        p->setPen( opt->palette.color( QPalette::Shadow ).light( 300 ) );
        p->setBrush( Qt::NoBrush );
        p->drawRect( opt->rect.adjusted( 0, 0, -1, -1 ) );
        }
        break;
    case CE_ProgressBarContents:
        {
        const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar*>(opt);
        if( pb ) {
            QRect rect = pb->rect;
            QRect progress;

            if( m_groovebuffer.isNull() || rect.size() != m_groovebuffer.size() ) {
                QColor color = opt->palette.color( QPalette::Shadow );
                m_groovebuffer = generate_progress_bar( color, color.light( 300 ), rect.size() );
            }

            if( pb->maximum > pb->minimum ) {
                progress = rect.adjusted( 0, 0,
                    -rect.width() + rect.width()*(pb->progress - pb->minimum)/(pb->maximum - pb->minimum), 0 );
                progress.adjust( 1, 1, -1, -1 );
            }

            if( progress.isValid() ) {
                if( opt->state == QStyle::State_Sunken ) {
                    if( m_silhouettebuffer.isNull() || rect.size() != m_silhouettebuffer.size() ) {
                        QColor color = opt->palette.color( QPalette::Highlight );
                        m_silhouettebuffer = generate_progress_bar( color, color.dark( 150 ), rect.size() );
                    }

                    p->drawPixmap( progress, m_silhouettebuffer );
                } else {
                    if( m_barbuffer.isNull() || rect.size() != m_barbuffer.size() ) {
                        QColor color = opt->palette.color( QPalette::Highlight );
                        m_barbuffer = generate_progress_bar( color, color.dark( 150 ), rect.size() );
                    }

                    p->setCompositionMode( QPainter::CompositionMode_Source ); // ### Cheat
                    p->drawPixmap( progress, m_barbuffer );
                    p->drawPixmap( QRect( QPoint( progress.right() + 1, rect.top() ),
                        QPoint( rect.right() - 1, rect.bottom() - 1 ) ), m_groovebuffer );
                }
            } else {
                p->setCompositionMode( QPainter::CompositionMode_Source ); // ### Cheat
                p->drawPixmap( rect.adjusted( 1, 1, -1, -1 ), m_groovebuffer );
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
