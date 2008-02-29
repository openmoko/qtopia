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

#include "slideshow.h"

#include <qtopiaapplication.h>


SlideShow::SlideShow( QObject* parent )
    : QObject( parent )
    , collection_model( &collection )
    , collection_i( -1 )
    , drm_content( QDrmRights::Display, QDrmContent::NoLicenseOptions )
{
    // When timeout advance to next image in collection
    connect( &timer, SIGNAL(timeout()), this, SLOT(advance()) );
    connect( this, SIGNAL(stopped()), &drm_content, SLOT(renderStopped()) );
}

SlideShow::~SlideShow()
{
    // Ensure backlight dimmer is re-enabled.
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);
}

void SlideShow::setFirstImage( const QContent& image )
{
    // Find image in collection and update iterator with this image

    for( collection_i = 0; collection_i < collection_model.rowCount(); collection_i++ )
    {
        if( collection_model.content( collection_i ).id() == image.id() )
        {
            if( (image.permissions() & QDrmRights::Automated) && drm_content.requestLicense( image ) )
            {
                drm_content.renderStarted();

                emit changed(image);
            }
            else
                QTimer::singleShot( 0, this, SLOT(advance()) );

            return;
        }
    }
}

void SlideShow::start()
{
    if ( collection.count() == 0 ) {
        // No slides to play.
        return;
    }

    // Disable power save while playing so that the device
    // doesn't suspend before the slides finish.
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Disable);

    timer.start( pause*FACTOR );
}

void SlideShow::stop()
{
    timer.stop();

    // Re-enable power save (that was disabled in start()).
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);

    emit stopped();
}

void SlideShow::advance()
{
    // If at end of collection and not looping through, stop slideshow
    // Otherwise, advance to next image in collection
    collection_i++;
    if (collection_i == collection.count() && !loop_through)
    {
        //timer.stop();
        //emit stopped();
        stop();
    }
    else
    {
        if (collection.count() > 1)
        {
            //if(collection_i == collection.count())
            //    collection_i = 0;
            if(collection_i == collection.count()) {
                // Wrap around - and put the power saving back on, just in case it wraps
                // around forever.
                collection_i = 0;
                QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);
            }

            QContent image = collection_model.content( collection_i );

            if( (image.permissions() & QDrmRights::Automated) && drm_content.requestLicense( image ) )
            {
                drm_content.renderStarted();

                emit changed(image);
            }
            else
                QTimer::singleShot( 0, this, SLOT(advance()) );

        }
    }
}
