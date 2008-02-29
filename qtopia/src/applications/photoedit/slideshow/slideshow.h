/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef SLIDESHOW_H
#define SLIDESHOW_H

#include <qtopia/applnk.h>

#include <qobject.h>
#include <qtimer.h>
#include <qvaluelist.h>

#define FACTOR 1000

class SlideShow : public QObject {
    Q_OBJECT
public:
    SlideShow( QObject* parent = 0, const char* name = 0 );
    
    // Set image collection
    void setCollection( const QValueList<DocLnk>& list ) { collection = list; }
    
    // Set first image to display
    void setFirstImage( const DocLnk& );
    
signals:
    // Current image has changed
    void changed( const DocLnk& );
    
    // Slide show has stopped
    void stopped();
    
public slots:
    // Start slide show
    void start() { timer.start( pause*FACTOR ); }
    
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
    
    QValueList<DocLnk> collection;
    QValueList<DocLnk>::ConstIterator collection_iterator;
};

#endif
