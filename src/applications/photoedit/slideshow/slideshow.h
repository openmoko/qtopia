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

#ifndef SLIDESHOW_H
#define SLIDESHOW_H

#include <qcontent.h>
#include <qcontentset.h>
#include <QDrmContent>
#include <qobject.h>
#include <qtimer.h>

#define FACTOR 1000

class SlideShow : public QObject {
    Q_OBJECT
public:
    SlideShow( QObject* parent = 0 );

    ~SlideShow();

    // Set image collection
    void setCollection( const QContentSet& list ) { collection = list; }

    // Set first image to display
    void setFirstImage( const QContent& );

signals:
    // Current image has changed
    void changed( const QContent& );

    // Slide show has stopped
    void stopped();

public slots:
    // Start slide show
    void start();

    // Stop slide show
    void stop();

    // If true, loop through collection
    void setLoopThrough( bool b ) { loop_through = b; }

    // Set pause between images in seconds
    void setSlideLength( int sec ) { pause = sec; }

private slots:
    // Advance to next image or emit finished
    void advance();

private:
    bool loop_through;
    int pause;
    QTimer timer;

    QContentSet collection;
    QContentSetModel collection_model;
    int collection_i;

    QDrmContent drm_content;
};

#endif
